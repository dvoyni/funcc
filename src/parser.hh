#pragma once

#include <format>
#include <functional>
#include <stdint.h>
#include <string>
#include <string_view>

#include "ast_common.hh"
#include "reader.hh"

#define skipWs()                     \
	if (m_ignoreWS) {                \
		m_ignoreWS->Consume(reader); \
	}

namespace funcc::parser {
	enum class ValueKind {
		Error,
		Exact,
		Ignore,
		WhiteSpace,
		SingleLineComment,
		MultiLineComment,
		Entity,
		StringLiteral,
		NumberLiteral,
		Multiple,
		SkippedOptional,
		Custom
	};

	class ITokenValue {
		ValueKind m_kind{ValueKind::Error};
		Range m_range{};

	public:
		ITokenValue(ValueKind kind, Range range) :
			m_kind{std::move(kind)},
			m_range{std::move(range)} {}

		virtual ~ITokenValue() = default;

		[[nodiscard]] ValueKind GetKind() const {
			return m_kind;
		};

		[[nodiscard]] Range const& GetRange() const {
			return m_range;
		}
	};

	class ErrorValue : public ITokenValue {
		std::string m_message;

	public:
		ErrorValue() :
			ITokenValue{ValueKind::Error, Range{}},
			m_message{} {};

		ErrorValue(Location start, IReader& reader, std::string&& message) :
			ITokenValue{ValueKind::Error, Range{std::move(start), reader.Pop()}} {
			m_message = std::format("{}; got `{}`", std::move(message), reader.Sub(GetRange()));
		}

		ErrorValue(std::string&& message) :
			ITokenValue{ValueKind::Error, Range{}},
			m_message{std::move(message)} {}

		~ErrorValue() = default;

		[[nodiscard]] std::string const& GetMessage() const {
			return m_message;
		}
	};

	class SimpleValue : public ITokenValue {
		std::string_view m_value;

	public:
		SimpleValue(ValueKind kind, Location start, IReader& reader) :
			ITokenValue{kind, Range{std::move(start), reader.GetLocation()}} {
			m_value = reader.Sub(GetRange());
		}

		~SimpleValue() = default;

		[[nodiscard]] std::string_view const& GetValue() const {
			return m_value;
		}
	};

	template<typename T>
	class Value : public ITokenValue {
		T m_value{};

	public:
		explicit Value(Range range, T const& arg) :
			ITokenValue{ValueKind::Custom, std::move(range)},
			m_value{arg} {}

		explicit Value(Range range, T&& arg) :
			ITokenValue{ValueKind::Custom, std::move(range)},
			m_value{std::forward<T>(arg)} {}

		~Value() = default;

		[[nodiscard]] T const& GetValue() const {
			return m_value;
		}
	};

	class MultiValue : public ITokenValue {
		std::vector<std::shared_ptr<ITokenValue>> m_values{};

	public:
		MultiValue(Location start, IReader& reader, std::vector<std::shared_ptr<ITokenValue>>&& values) :
			ITokenValue{ValueKind::Multiple, Range{std::move(start), reader.GetLocation()}},
			m_values{std::move(values)} {}

		~MultiValue() = default;

		[[nodiscard]] std::vector<std::shared_ptr<ITokenValue>> const& GetValues() const {
			return m_values;
		}
	};

	class IToken {
	public:
		virtual ~IToken() = default;
		virtual std::shared_ptr<ITokenValue> Consume(IReader& reader) const = 0;
	};

	class ExactToken : public IToken {
		std::string_view m_target;
		std::shared_ptr<IToken> m_ignoreWS = nullptr;

	public:
		ExactToken(std::string_view target, std::shared_ptr<IToken> ignoreWS = nullptr) :
			m_target{std::move(target)},
			m_ignoreWS{std::move(ignoreWS)} {}

		~ExactToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			skipWs();
			Location start = reader.Push();
			size_t length = m_target.length();
			for (size_t i = 0; i < length; ++i) {
				if (reader.GetChar() != m_target[i] || !reader.Move()) {
					return std::make_shared<ErrorValue>(start, reader, std::format("Expected '{}'", m_target));
				}
			}
			return std::make_shared<SimpleValue>(ValueKind::Exact, start, reader);
		}

		[[nodiscard]] std::string_view GetTarget() const {
			return m_target;
		}
	};

	inline static std::shared_ptr<IToken> Exact(std::string_view target, std::shared_ptr<IToken> ignoreWS = nullptr) {
		return std::make_shared<ExactToken>(std::move(target), std::move(ignoreWS));
	}

	class IgnoreAnyToken : public IToken {
		std::vector<std::shared_ptr<IToken>> m_tokens;
		std::shared_ptr<IToken> m_ignoreWS = nullptr;

	public:
		IgnoreAnyToken(std::vector<std::shared_ptr<IToken>>&& tokens, std::shared_ptr<IToken> ignoreWS = nullptr) :
			m_tokens{std::move(tokens)},
			m_ignoreWS{std::move(ignoreWS)} {}

		~IgnoreAnyToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			skipWs();
			Location start = reader.GetLocation();

			bool consumed = true;
			while (consumed) {
				consumed = false;
				for (auto& token: m_tokens) {
					skipWs();
					std::shared_ptr<ITokenValue> result = token->Consume(reader);
					if (result->GetKind() != ValueKind::Error) {
						consumed = true;
						break;
					}
				}
			}
			return std::make_shared<SimpleValue>(ValueKind::Ignore, start, reader);
		}
	};

	inline static std::shared_ptr<IToken> IgnoreAny(
		std::vector<std::shared_ptr<IToken>>&& tokens,
		std::shared_ptr<IToken> ignoreWS = nullptr
	) {
		return std::make_shared<IgnoreAnyToken>(std::move(tokens), std::move(ignoreWS));
	}

	class OneOfToken : public IToken {
		std::vector<std::shared_ptr<IToken>> m_tokens;
		std::shared_ptr<IToken> m_ignoreWS = nullptr;

	public:
		OneOfToken(std::vector<std::shared_ptr<IToken>>&& tokens, std::shared_ptr<IToken> ignoreWS = nullptr) :
			m_tokens{std::move(tokens)},
			m_ignoreWS{std::move(ignoreWS)} {}

		~OneOfToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			std::shared_ptr<ITokenValue> furthestError{};
			skipWs();
			for (auto& token: m_tokens) {
				std::shared_ptr<ITokenValue> result = token->Consume(reader);
				if (result) {
					return result;
				}
				if (!furthestError || furthestError->GetRange() < result->GetRange()) {
					furthestError = result;
				}
			}
			return furthestError;
		}
	};

	inline static std::shared_ptr<IToken> OneOf(
		std::vector<std::shared_ptr<IToken>>&& tokens,
		std::shared_ptr<IToken> ignoreWS = nullptr
	) {
		return std::make_shared<OneOfToken>(std::move(tokens), std::move(ignoreWS));
	}

	class AllToken : public IToken {
		std::vector<std::shared_ptr<IToken>> m_tokens;
		std::shared_ptr<IToken> m_ignoreWS = nullptr;
		std::function<bool(std::shared_ptr<ITokenValue> const&)> m_filter;

		inline static bool FilterIgnored(std::shared_ptr<ITokenValue> const& value) {
			return value->GetKind() != ValueKind::Ignore;
		}

	public:
		AllToken(
			std::vector<std::shared_ptr<IToken>>&& tokens,
			std::shared_ptr<IToken> ignoreWS = nullptr,
			std::function<bool(std::shared_ptr<ITokenValue> const&)> filter = FilterIgnored
		) :
			m_tokens{std::move(tokens)},
			m_ignoreWS{std::move(ignoreWS)},
			m_filter{std::move(filter)} {}

		~AllToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			std::vector<std::shared_ptr<ITokenValue>> results{};
			Location start = reader.Push();
			for (auto& token: m_tokens) {
				std::shared_ptr<ITokenValue> result = token->Consume(reader);
				if (result->GetKind() == ValueKind::Error) {
					reader.Pop();
					return result;
				}

				if (m_filter(result)) {
					results.push_back(result);
				}
			}
			return std::make_shared<MultiValue>(start, reader, std::move(results));
		}
	};

	inline static std::shared_ptr<IToken> All(
		std::vector<std::shared_ptr<IToken>>&& tokens,
		std::shared_ptr<IToken> ignoreWS = nullptr,
		std::function<bool(std::shared_ptr<ITokenValue> const&)> filter = nullptr
	) {
		return std::make_shared<AllToken>(std::move(tokens), std::move(ignoreWS), std::move(filter));
	}

	class OptionalToken : public IToken {
		std::shared_ptr<IToken> m_token;
		std::shared_ptr<IToken> m_dependent{};

	public:
		OptionalToken(std::shared_ptr<IToken> token, std::shared_ptr<IToken> dependent = nullptr) :
			m_token{std::move(token)},
			m_dependent{std::move(dependent)} {}

		~OptionalToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.Push();
			std::shared_ptr<ITokenValue> result = m_token->Consume(reader);
			if (result->GetKind() == ValueKind::Error) {
				return std::make_shared<SimpleValue>(ValueKind::SkippedOptional, reader.GetLocation(), reader);
			}
			if (!m_dependent) {
				return result;
			}
			std::shared_ptr<ITokenValue> value = m_dependent->Consume(reader);
			return std::make_shared<MultiValue>(
				start,
				reader,
				std::vector<std::shared_ptr<ITokenValue>>{result, value}
			);
		}
	};

	inline static std::shared_ptr<IToken> Optional(
		std::shared_ptr<IToken> token,
		std::shared_ptr<IToken> dependent = nullptr
	) {
		return std::make_shared<OptionalToken>(std::move(token), std::move(dependent));
	}

	class SomeToken : public IToken {
		std::shared_ptr<IToken> m_prefix;
		std::shared_ptr<IToken> m_item;
		std::shared_ptr<IToken> m_separator;
		std::shared_ptr<IToken> m_suffix;
		std::shared_ptr<IToken> m_ignoreWS{nullptr};
		bool m_allowEmpty{false};
		bool m_allowSeparatorBeforeSuffix{false};

	public:
		SomeToken(
			std::shared_ptr<IToken> prefix,
			std::shared_ptr<IToken> item,
			std::shared_ptr<IToken> separator,
			std::shared_ptr<IToken> suffix,
			std::shared_ptr<IToken> ignoreWS = nullptr,
			bool allowEmpty = false,
			bool allowSeparatorBeforeSuffix = false
		) :
			m_prefix{std::move(prefix)},
			m_item{std::move(item)},
			m_separator{std::move(separator)},
			m_suffix{std::move(suffix)},
			m_ignoreWS{std::move(ignoreWS)},
			m_allowEmpty{allowEmpty},
			m_allowSeparatorBeforeSuffix{allowSeparatorBeforeSuffix} {}

		~SomeToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			skipWs();
			Location start = reader.Push();
			std::shared_ptr<ITokenValue> prefix = m_prefix->Consume(reader);
			if (prefix->GetKind() == ValueKind::Error) {
				reader.Pop();
				return prefix;
			}

			std::vector<std::shared_ptr<ITokenValue>> values{};
			while (true) {
				skipWs();
				std::shared_ptr<ITokenValue> item = m_item->Consume(reader);
				std::shared_ptr<ITokenValue> separator = m_separator->Consume(reader);
				std::shared_ptr<ITokenValue> suffix = m_suffix->Consume(reader);

				if (item->GetKind() != ValueKind::Error) {
					values.push_back(item);
					if (suffix->GetKind() != ValueKind::Error) {
						if (separator->GetKind() != ValueKind::Error && !m_allowSeparatorBeforeSuffix) {
							reader.Pop();
							return std::make_shared<ErrorValue>(start, reader, "Did not expect separator here");
						}

						break;
					}
				} else if (suffix->GetKind() != ValueKind::Error) {
					if (!m_allowEmpty && values.empty()) {
						reader.Pop();
						return std::make_shared<ErrorValue>(start, reader, "Expected an item");
					}
				} else if (!reader.Move()) {
					reader.Pop();
					return suffix;
				}
			}

			return std::make_shared<MultiValue>(start, reader, std::move(values));
		}
	};

	inline static std::shared_ptr<IToken> Some(
		std::shared_ptr<IToken> prefix,
		std::shared_ptr<IToken> item,
		std::shared_ptr<IToken> separator,
		std::shared_ptr<IToken> suffix,
		std::shared_ptr<IToken> ignoreWS = nullptr,
		bool allowEmpty = false,
		bool allowSeparatorBeforeSuffix = false
	) {
		return std::make_shared<SomeToken>(
			std::move(prefix),
			std::move(item),
			std::move(separator),
			std::move(suffix),
			std::move(ignoreWS),
			allowEmpty,
			allowSeparatorBeforeSuffix
		);
	}

	class WhiteSpaceToken : public IToken {
	public:
		WhiteSpaceToken() = default;

		~WhiteSpaceToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.Push();
			while (true) {
				uint32_t c = reader.GetChar();
				if (!std::isspace(c) || !reader.Move()) {
					break;
				}
			}
			if (start < reader.GetLocation()) {
				return std::make_shared<SimpleValue>(ValueKind::WhiteSpace, start, reader);
			}
			return std::make_shared<ErrorValue>(start, reader, "Expected whitespace");
		}
	};

	inline static std::shared_ptr<IToken> WhiteSpace() {
		return std::make_shared<WhiteSpaceToken>();
	}

	class SingleLineCommentToken : public IToken {
		ExactToken m_prefix;

	public:
		SingleLineCommentToken(std::string_view prefix) :
			m_prefix{std::move(prefix)} {}

		~SingleLineCommentToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			ErrorValue ignore{};
			Location start = reader.Push();

			std::shared_ptr<ITokenValue> prefix = m_prefix.Consume(reader);
			if (prefix->GetKind() == ValueKind::Error) {
				reader.Pop();
				return prefix;
			}

			while (true) {
				uint32_t c = reader.GetChar();
				if (c == '\n' || !reader.Move()) {
					break;
				}
			}

			return std::make_shared<SimpleValue>(ValueKind::SingleLineComment, start, reader);
		}
	};

	inline static std::shared_ptr<IToken> SingleLineComment(std::string_view prefix) {
		return std::make_shared<SingleLineCommentToken>(std::move(prefix));
	}

	class MultiLineCommentToken : public IToken {
		ExactToken m_prefix;
		ExactToken m_suffix;

	public:
		MultiLineCommentToken(std::string_view prefix, std::string_view suffix) :
			m_prefix{std::move(prefix)},
			m_suffix{std::move(suffix)} {}

		~MultiLineCommentToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.Push();
			std::shared_ptr<ITokenValue> prefix = m_prefix.Consume(reader);
			if (prefix->GetKind() == ValueKind::Error) {
				reader.Pop();
				return prefix;
			}

			while (true) {
				std::shared_ptr<ITokenValue> suffix = m_suffix.Consume(reader);
				if (suffix->GetKind() != ValueKind::Error) {
					break;
				}
				if (!reader.Move()) {
					reader.Pop();
					return suffix;
				}
			}

			return std::make_shared<SimpleValue>(ValueKind::MultiLineComment, start, reader);
		}
	};

	inline static std::shared_ptr<IToken> MultiLineComment(std::string_view prefix, std::string_view suffix) {
		return std::make_shared<MultiLineCommentToken>(std::move(prefix), std::move(suffix));
	}

	class EntityToken : public IToken {
	public:
		using Aggregator =
			std::function<void(std::string_view const& acc, uint32_t next, bool& outIsValid, bool& outIsComplete)>;
		Aggregator m_aggregator;
		std::shared_ptr<IToken> m_ignoreWS = nullptr;

	public:
		EntityToken(Aggregator aggregator, std::shared_ptr<IToken> ignoreWS = nullptr) :
			m_aggregator{std::move(aggregator)},
			m_ignoreWS{std::move(ignoreWS)} {}

		~EntityToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			skipWs();
			Location start = reader.Push();
			while (true) {
				uint32_t c = reader.GetChar();
				bool isValid = false;
				bool isComplete = false;
				m_aggregator(reader.Sub(Range{start, reader.GetLocation()}), c, isValid, isComplete);
				if (isComplete) {
					if (isValid) {
						return std::make_shared<SimpleValue>(ValueKind::Entity, start, reader);
					} else {
						return std::make_shared<ErrorValue>(start, reader, "Invalid identifier");
					}
				}
				if (!reader.Move()) {
					return std::make_shared<ErrorValue>(start, reader, "Invalid identifier");
				}
			}
		}
	};

	inline static std::shared_ptr<IToken> Entity(
		EntityToken::Aggregator aggregator,
		std::shared_ptr<IToken> ignoreWS = nullptr
	) {
		return std::make_shared<EntityToken>(std::move(aggregator), std::move(ignoreWS));
	}

	class StringLiteralToken : public IToken {
		ExactToken m_prefix;
		ExactToken m_suffix;
		ExactToken m_escape;
		std::string_view m_value{};

	public:
		StringLiteralToken(std::string_view prefix, std::string_view suffix, std::string_view escape) :
			m_prefix{std::move(prefix)},
			m_suffix{std::move(suffix)},
			m_escape{std::move(escape)} {}

		~StringLiteralToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.Push();
			std::shared_ptr<ITokenValue> result = m_prefix.Consume(reader);
			if (result->GetKind() == ValueKind::Error) {
				reader.Pop();
				return result;
			}

			bool escaped = false;
			while (true) {
				if (!escaped) {
					if (m_escape.Consume(reader)->GetKind() != ValueKind::Error) {
						escaped = true;
					}
				}
				std::shared_ptr<ITokenValue> suffix = m_suffix.Consume(reader);
				if (!escaped && suffix->GetKind() != ValueKind::Error) {
					break;
				}
				if (!reader.Move()) {
					reader.Pop();
					return suffix;
				}
			}
			return std::make_shared<SimpleValue>(ValueKind::StringLiteral, start, reader);
		}
	};

	inline static std::shared_ptr<IToken> StringLiteral(
		std::string_view prefix,
		std::string_view suffix,
		std::string_view escape
	) {
		return std::make_shared<StringLiteralToken>(std::move(prefix), std::move(suffix), std::move(escape));
	}

	class NumberLiteralToken : public IToken {
		std::string_view m_value{};

	public:
		NumberLiteralToken() = default;
		~NumberLiteralToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.Push();

			char const* begin = reader.Sub(Range{reader.GetLocation(), reader.GetLocation()}).data();
			char* end = nullptr;
			double val = strtod(begin, &end);

			if (val == HUGE_VAL || end == begin) {
				return std::make_shared<ErrorValue>(start, reader, "Expected number");
			}

			size_t len = end - begin;
			while (len--) {
				if (!reader.Move()) {
					return std::make_shared<ErrorValue>(start, reader, "Expected number");
				}
			}

			return std::make_shared<SimpleValue>(ValueKind::NumberLiteral, start, reader);
		}
	};

	inline static std::shared_ptr<IToken> NumberLiteral() {
		return std::make_shared<NumberLiteralToken>();
	}

	class MapToken : public IToken {
	public:
		using Mapper = std::function<std::shared_ptr<ITokenValue>(std::shared_ptr<ITokenValue> const& value)>;

	private:
		Mapper m_mapper;
		std::shared_ptr<IToken> m_token;

	public:
		MapToken(std::shared_ptr<IToken> token, Mapper mapper) :
			m_mapper{std::move(mapper)},
			m_token{std::move(token)} {}

		~MapToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			std::shared_ptr<ITokenValue> result = m_token->Consume(reader);
			if (result->GetKind() == ValueKind::Error) {
				return result;
			}
			return m_mapper(result);
		}
	};

	inline static std::shared_ptr<IToken> Map(std::shared_ptr<IToken> token, MapToken::Mapper mapper) {
		return std::make_shared<MapToken>(std::move(token), std::move(mapper));
	}
}
