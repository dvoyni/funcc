#pragma once

#include "_external.hh"
#include "ast_common.hh"
#include "reader.hh"

#define skipWs()                     \
	if (m_ignoreWS) {                \
		m_ignoreWS->Consume(reader); \
	}

#define FUNCC_DEBUG_TOKEN ""

#define FUNCC_DEBUG_TOKEN_BREAK(name)           \
	if (strcmp(name, FUNCC_DEBUG_TOKEN) == 0) { \
		__debugbreak();                         \
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

		[[nodiscard]] bool HasValue() const {
			return m_kind != ValueKind::Error;
		}

		[[nodiscard]] bool HasError() const {
			return m_kind == ValueKind::Error;
		}

		[[nodiscard]] bool IsSkipped() const {
			return m_kind == ValueKind::SkippedOptional;
		}

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

		ErrorValue(Range range, std::string&& message) :
			ITokenValue{ValueKind::Error, std::move(range)},
			m_message{std::move(message)} {}

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

	class NumberLiteralValue : public ITokenValue {
		bool m_isInteger;
		int64_t m_integer;
		bool m_isFloat;
		double m_float;

	public:
		NumberLiteralValue(Location start, IReader& reader) :
			ITokenValue{ValueKind::NumberLiteral, Range{std::move(start), reader.GetLocation()}} {
			std::string_view std = reader.Sub(GetRange());

			char* endPtr = nullptr;
			m_float = strtod(std.data(), &endPtr);
			m_isFloat = endPtr == std.data() + std.length();

			m_integer = strtoll(std.data(), &endPtr, 10);
			m_isInteger = endPtr == std.data() + std.length();
		}

		~NumberLiteralValue() = default;

		[[nodiscard]] bool IsInteger() const {
			return m_isInteger;
		}

		[[nodiscard]] int64_t GetInteger() const {
			return m_integer;
		}

		[[nodiscard]] bool IsFloat() const {
			return m_isFloat;
		}

		[[nodiscard]] double GetFloat() const {
			return m_float;
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

		template<typename T>
		[[nodiscard]] std::vector<T> Extract() const {
			return Extract<T>([](std::shared_ptr<ITokenValue> const& value) {
				return std::dynamic_pointer_cast<Value<T>>(value)->GetValue();
			});
		}

		template<typename T>
		[[nodiscard]] std::vector<T> Extract(std::function<T(std::shared_ptr<ITokenValue> const&)> extractor) const {
			std::vector<T> result{};
			for (auto& value: m_values) {
				result.push_back(extractor(value));
			}
			return result;
		}
	};

	class IToken {
	public:
		virtual ~IToken() = default;
		virtual std::shared_ptr<ITokenValue> Consume(IReader& reader) const = 0;

	protected:
		std::shared_ptr<ITokenValue> RewindWithError(Location start, IReader& reader, std::string_view message) const {
			Range range{start, reader.GetLocation()};
			reader.SetLocation(start);
			return std::make_shared<ErrorValue>(range, std::string(message));
		}
	};

	class ExactToken : public IToken {
		std::string_view m_target;
		std::shared_ptr<IToken> m_ignoreWS;

	public:
		ExactToken(std::string_view target, std::shared_ptr<IToken> ignoreWS) :
			m_target{std::move(target)},
			m_ignoreWS{std::move(ignoreWS)} {}

		~ExactToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.GetLocation();
			skipWs();

			size_t length = m_target.length();
			for (size_t i = 0; i < length; ++i) {
				if (reader.GetChar() != m_target[i] || !reader.Move()) {
					return RewindWithError(
						start,
						reader,
						std::string("Expected '") + std::string(m_target) + std::string("'")  // TODO: make it better?
					);
				}
			}
			return std::make_shared<SimpleValue>(ValueKind::Exact, start, reader);
		}

		[[nodiscard]] std::string_view GetTarget() const {
			return m_target;
		}
	};

	inline static std::shared_ptr<IToken> Exact(std::string_view target, std::shared_ptr<IToken> ignoreWS) {
		return std::make_shared<ExactToken>(std::move(target), std::move(ignoreWS));
	}

	class IgnoreAnyToken : public IToken {
		std::vector<std::shared_ptr<IToken>> m_tokens;
		std::shared_ptr<IToken> m_ignoreWS;

	public:
		IgnoreAnyToken(std::vector<std::shared_ptr<IToken>>&& tokens, std::shared_ptr<IToken> ignoreWS) :
			m_tokens{std::move(tokens)},
			m_ignoreWS{std::move(ignoreWS)} {}

		~IgnoreAnyToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.GetLocation();
			skipWs();

			bool consumed = true;
			while (consumed) {
				consumed = false;
				for (auto& token: m_tokens) {
					skipWs();
					std::shared_ptr<ITokenValue> result = token->Consume(reader);
					if (result->HasValue()) {
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
		std::shared_ptr<IToken> ignoreWS
	) {
		return std::make_shared<IgnoreAnyToken>(std::move(tokens), std::move(ignoreWS));
	}

	class OneOfToken : public IToken {
		std::vector<std::shared_ptr<IToken>> m_tokens;
		std::shared_ptr<IToken> m_ignoreWS;

	public:
		OneOfToken(std::vector<std::shared_ptr<IToken>>&& tokens, std::shared_ptr<IToken> ignoreWS) :
			m_tokens{std::move(tokens)},
			m_ignoreWS{std::move(ignoreWS)} {}

		~OneOfToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.GetLocation();
			skipWs();

			std::shared_ptr<ITokenValue> furthestError{};

			for (auto& token: m_tokens) {
				std::shared_ptr<ITokenValue> result = token->Consume(reader);
				if (result->HasValue()) {
					return result;
				}
				if (!furthestError || furthestError->GetRange() < result->GetRange()) {
					furthestError = result;
				}
			}

			reader.SetLocation(start);
			return furthestError;
		}
	};

	inline static std::shared_ptr<IToken> OneOf(
		std::vector<std::shared_ptr<IToken>>&& tokens,
		std::shared_ptr<IToken> ignoreWS
	) {
		return std::make_shared<OneOfToken>(std::move(tokens), std::move(ignoreWS));
	}

	class AllToken : public IToken {
		std::vector<std::shared_ptr<IToken>> m_tokens;
		std::shared_ptr<IToken> m_ignoreWS;
		std::function<bool(std::shared_ptr<ITokenValue> const&)> m_filter;

	public:
		inline static bool FilterIgnored(std::shared_ptr<ITokenValue> const& value) {
			return value->GetKind() != ValueKind::Ignore;
		}

		AllToken(
			std::vector<std::shared_ptr<IToken>>&& tokens,
			std::shared_ptr<IToken> ignoreWS,
			std::function<bool(std::shared_ptr<ITokenValue> const&)> filter = FilterIgnored
		) :
			m_tokens{std::move(tokens)},
			m_ignoreWS{std::move(ignoreWS)},
			m_filter{std::move(filter)} {}

		~AllToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.GetLocation();

			std::vector<std::shared_ptr<ITokenValue>> results{};

			for (auto& token: m_tokens) {
				std::shared_ptr<ITokenValue> result = token->Consume(reader);
				if (!result->HasValue()) {
					reader.SetLocation(start);
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
		std::shared_ptr<IToken> ignoreWS,
		std::function<bool(std::shared_ptr<ITokenValue> const&)> filter = &AllToken::FilterIgnored
	) {
		return std::make_shared<AllToken>(std::move(tokens), std::move(ignoreWS), std::move(filter));
	}

	class OptionalToken : public IToken {
		std::shared_ptr<IToken> m_token;
		std::shared_ptr<IToken> m_dependent{};
		std::shared_ptr<IToken> m_alternative{};

	public:
		OptionalToken(
			std::shared_ptr<IToken> token,
			std::shared_ptr<IToken> dependent = nullptr,
			std::shared_ptr<IToken> alternative = nullptr
		) :
			m_token{std::move(token)},
			m_dependent{std::move(dependent)},
			m_alternative{std::move(alternative)} {}

		~OptionalToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.GetLocation();

			std::shared_ptr<ITokenValue> result = m_token->Consume(reader);
			if (result->HasError()) {
				if (m_alternative) {
					std::shared_ptr<ITokenValue> altValue = m_alternative->Consume(reader);
					if (altValue->HasError()) {
						reader.SetLocation(start);
					}
					return altValue;
				}
				return std::make_shared<SimpleValue>(ValueKind::SkippedOptional, reader.GetLocation(), reader);
			}
			if (!m_dependent) {
				return result;
			}
			std::shared_ptr<ITokenValue> value = m_dependent->Consume(reader);
			if (value->HasError()) {
				reader.SetLocation(start);
			}
			return value;
		}
	};

	inline static std::shared_ptr<IToken> Optional(
		std::shared_ptr<IToken> token,
		std::shared_ptr<IToken> dependent = nullptr,
		std::shared_ptr<IToken> alternative = nullptr
	) {
		return std::make_shared<OptionalToken>(std::move(token), std::move(dependent), std::move(alternative));
	}

	class SomeToken : public IToken {
		std::shared_ptr<IToken> m_item;
		std::shared_ptr<IToken> m_prefix;
		std::shared_ptr<IToken> m_suffix;
		std::shared_ptr<IToken> m_separator;
		std::shared_ptr<IToken> m_ignoreWS;
		std::shared_ptr<IToken> m_firstItem;
		bool m_allowEmpty;
		bool m_allowSeparatorBeforeSuffix;

	public:
		SomeToken(
			std::shared_ptr<IToken> item,
			std::shared_ptr<IToken> prefix,
			std::shared_ptr<IToken> suffix,
			std::shared_ptr<IToken> separator,
			std::shared_ptr<IToken> ignoreWS,
			std::shared_ptr<IToken> firstItem = nullptr,
			bool allowEmpty = false,
			bool allowSeparatorBeforeSuffix = false
		) :
			m_item{std::move(item)},
			m_prefix{std::move(prefix)},
			m_suffix{std::move(suffix)},
			m_separator{std::move(separator)},
			m_ignoreWS{std::move(ignoreWS)},
			m_firstItem{std::move(firstItem)},
			m_allowEmpty{allowEmpty},
			m_allowSeparatorBeforeSuffix{allowSeparatorBeforeSuffix} {}

		~SomeToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.GetLocation();
			skipWs();

			if (m_prefix) {
				std::shared_ptr<ITokenValue> prefix = m_prefix->Consume(reader);
				if (prefix->HasError()) {
					reader.SetLocation(start);
					return prefix;
				}
			}

			std::vector<std::shared_ptr<ITokenValue>> values{};
			bool first = true;
			while (true) {
				skipWs();

				std::shared_ptr<ITokenValue> separator = m_separator->Consume(reader);
				std::shared_ptr<ITokenValue> suffix = nullptr;

				if (separator->HasError() || m_allowSeparatorBeforeSuffix || !first || m_allowEmpty) {
					skipWs();
					suffix = m_suffix->Consume(reader);
				}

				if (suffix && suffix->HasValue()) {
					break;
				}

				skipWs();
				std::shared_ptr<ITokenValue> item = ((first && m_firstItem) ? m_firstItem : m_item)->Consume(reader);
				if (item->HasError()) {
					reader.SetLocation(start);
					return item;
				}

				values.push_back(item);
				first = false;
			}

			return std::make_shared<MultiValue>(start, reader, std::move(values));
		}
	};

	inline static std::shared_ptr<IToken> Some(
		std::shared_ptr<IToken> item,
		std::shared_ptr<IToken> prefix,
		std::shared_ptr<IToken> suffix,
		std::shared_ptr<IToken> separator,
		std::shared_ptr<IToken> ignoreWS,
		std::shared_ptr<IToken> firstItem = nullptr,
		bool allowEmpty = false,
		bool allowSeparatorBeforeSuffix = false
	) {
		return std::make_shared<SomeToken>(
			std::move(item),
			std::move(prefix),
			std::move(suffix),
			std::move(separator),
			std::move(ignoreWS),
			std::move(firstItem),
			allowEmpty,
			allowSeparatorBeforeSuffix
		);
	}

	class RepeatToken : public IToken {
		std::shared_ptr<IToken> m_condition;
		std::shared_ptr<IToken> m_body;
		std::shared_ptr<IToken> m_ignoreWS;
		bool m_allowEmpty;

	public:
		RepeatToken(
			std::shared_ptr<IToken> condition,
			std::shared_ptr<IToken> body,
			std::shared_ptr<IToken> ignoreWS,
			bool allowEmpty = false
		) :
			m_condition{std::move(condition)},
			m_body{std::move(body)},
			m_ignoreWS{std::move(ignoreWS)},
			m_allowEmpty{allowEmpty} {}

		~RepeatToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.GetLocation();
			skipWs();

			std::vector<std::shared_ptr<ITokenValue>> values{};
			while (true) {
				Location itemStart = reader.GetLocation();
				skipWs();

				std::shared_ptr<ITokenValue> condition = m_condition->Consume(reader);
				reader.SetLocation(itemStart);

				if (condition->HasError()) {
					if (!m_allowEmpty && values.empty()) {
						reader.SetLocation(start);
						return condition;
					}
					break;
				}
				skipWs();
				std::shared_ptr<ITokenValue> body = m_body->Consume(reader);
				if (body->HasError()) {
					reader.SetLocation(start);
					return body;
				}
				values.push_back(body);
			}
			return std::make_shared<MultiValue>(start, reader, std::move(values));
		}
	};

	inline static std::shared_ptr<IToken> Repeat(
		std::shared_ptr<IToken> condition,
		std::shared_ptr<IToken> body,
		std::shared_ptr<IToken> ignoreWS,
		bool allowEmpty = false
	) {
		return std::make_shared<RepeatToken>(std::move(condition), std::move(body), std::move(ignoreWS), allowEmpty);
	}

	class WhiteSpaceToken : public IToken {
	public:
		WhiteSpaceToken() = default;

		~WhiteSpaceToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.GetLocation();

			while (true) {
				uint32_t c = reader.GetChar();
				if (!std::isspace(c) || !reader.Move()) {
					break;
				}
			}
			if (start < reader.GetLocation()) {
				return std::make_shared<SimpleValue>(ValueKind::WhiteSpace, start, reader);
			}
			return RewindWithError(start, reader, "Expected whitespace");
		}
	};

	inline static std::shared_ptr<IToken> WhiteSpace() {
		return std::make_shared<WhiteSpaceToken>();
	}

	class SingleLineCommentToken : public IToken {
		ExactToken m_prefix;

	public:
		SingleLineCommentToken(std::string_view prefix, std::shared_ptr<IToken> ignoreWS) :
			m_prefix{std::move(prefix), std::move(ignoreWS)} {}

		~SingleLineCommentToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.GetLocation();

			ErrorValue ignore{};

			std::shared_ptr<ITokenValue> prefix = m_prefix.Consume(reader);
			if (prefix->HasError()) {
				reader.SetLocation(start);
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

	inline static std::shared_ptr<IToken> SingleLineComment(std::string_view prefix, std::shared_ptr<IToken> ignoreWS) {
		return std::make_shared<SingleLineCommentToken>(std::move(prefix), std::move(ignoreWS));
	}

	class MultiLineCommentToken : public IToken {
		ExactToken m_prefix;
		ExactToken m_suffix;

	public:
		MultiLineCommentToken(std::string_view prefix, std::string_view suffix, std::shared_ptr<IToken> ignoreWS) :
			m_prefix{std::move(prefix), ignoreWS},
			m_suffix{std::move(suffix), ignoreWS} {}

		~MultiLineCommentToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.GetLocation();

			std::shared_ptr<ITokenValue> prefix = m_prefix.Consume(reader);
			if (prefix->HasError()) {
				reader.SetLocation(start);
				return prefix;
			}

			while (true) {
				std::shared_ptr<ITokenValue> suffix = m_suffix.Consume(reader);
				if (suffix->HasValue()) {
					break;
				}
				if (!reader.Move()) {
					reader.SetLocation(start);
					return suffix;
				}
			}

			return std::make_shared<SimpleValue>(ValueKind::MultiLineComment, start, reader);
		}
	};

	inline static std::shared_ptr<IToken> MultiLineComment(
		std::string_view prefix,
		std::string_view suffix,
		std::shared_ptr<IToken> ignoreWS
	) {
		return std::make_shared<MultiLineCommentToken>(std::move(prefix), std::move(suffix), std::move(ignoreWS));
	}

	class EntityToken : public IToken {
	public:
		using Aggregator =
			std::function<void(std::string_view const& acc, uint32_t next, bool& outIsValid, bool& outIsComplete)>;
		Aggregator m_aggregator;
		std::shared_ptr<IToken> m_ignoreWS;

	public:
		EntityToken(Aggregator aggregator, std::shared_ptr<IToken> ignoreWS) :
			m_aggregator{std::move(aggregator)},
			m_ignoreWS{std::move(ignoreWS)} {}

		~EntityToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.GetLocation();
			skipWs();

			while (true) {
				uint32_t c = reader.GetChar();
				bool isValid = false;
				bool isComplete = false;
				m_aggregator(reader.Sub(Range{start, reader.GetLocation()}), c, isValid, isComplete);
				if (isComplete) {
					if (isValid) {
						return std::make_shared<SimpleValue>(ValueKind::Entity, start, reader);
					} else {
						return RewindWithError(start, reader, "Invalid identifier");
					}
				}
				if (!reader.Move()) {
					return RewindWithError(start, reader, "Invalid identifier");
				}
			}
		}
	};

	inline static std::shared_ptr<IToken> Entity(EntityToken::Aggregator aggregator, std::shared_ptr<IToken> ignoreWS) {
		return std::make_shared<EntityToken>(std::move(aggregator), std::move(ignoreWS));
	}

	class StringLiteralToken : public IToken {
		ExactToken m_prefix;
		ExactToken m_suffix;
		ExactToken m_escape;
		std::string_view m_value{};

	public:
		StringLiteralToken(
			std::string_view prefix,
			std::string_view suffix,
			std::string_view escape,
			std::shared_ptr<IToken> ignoreWS
		) :
			m_prefix{std::move(prefix), ignoreWS},
			m_suffix{std::move(suffix), nullptr},
			m_escape{std::move(escape), nullptr} {}

		~StringLiteralToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.GetLocation();

			std::shared_ptr<ITokenValue> result = m_prefix.Consume(reader);
			if (result->HasError()) {
				reader.SetLocation(start);
				return result;
			}

			bool escaped = false;
			while (true) {
				if (!escaped) {
					if (m_escape.Consume(reader)->HasValue()) {
						escaped = true;
					}
				}
				std::shared_ptr<ITokenValue> suffix = m_suffix.Consume(reader);
				if (!escaped && suffix->HasValue()) {
					break;
				}
				if (!reader.Move()) {
					reader.SetLocation(start);
					return suffix;
				}
			}
			return std::make_shared<SimpleValue>(ValueKind::StringLiteral, start, reader);
		}
	};

	inline static std::shared_ptr<IToken> StringLiteral(
		std::string_view prefix,
		std::string_view suffix,
		std::string_view escape,
		std::shared_ptr<IToken> ignoreWS
	) {
		return std::make_shared<StringLiteralToken>(
			std::move(prefix),
			std::move(suffix),
			std::move(escape),
			std::move(ignoreWS)
		);
	}

	class NumberLiteralToken : public IToken {
		std::string_view m_value{};
		std::shared_ptr<IToken> m_ignoreWS;

	public:
		NumberLiteralToken(std::shared_ptr<IToken> ignoreWS) :
			m_ignoreWS{std::move(ignoreWS)} {}

		~NumberLiteralToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			Location start = reader.GetLocation();
			skipWs();

			char const* begin = reader.Sub(Range{reader.GetLocation(), reader.GetLocation()}).data();
			char* end = nullptr;
			double val = strtod(begin, &end);

			if (val == HUGE_VAL || end == begin) {
				return RewindWithError(start, reader, "Expected number");
			}

			size_t len = end - begin;
			while (len--) {
				if (!reader.Move()) {
					return RewindWithError(start, reader, "Expected number");
				}
			}

			return std::make_shared<NumberLiteralValue>(start, reader);
		}
	};

	inline static std::shared_ptr<IToken> NumberLiteral(std::shared_ptr<IToken> ignoreWS) {
		return std::make_shared<NumberLiteralToken>(ignoreWS);
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
			if (result->HasError()) {
				return result;
			}
			return m_mapper(result);
		}
	};

	inline static std::shared_ptr<IToken> Map(std::shared_ptr<IToken> token, MapToken::Mapper mapper) {
		return std::make_shared<MapToken>(std::move(token), std::move(mapper));
	}

	class EOFToken : public IToken {
		std::shared_ptr<IToken> m_ignoreWS;

	public:
		EOFToken(std::shared_ptr<IToken> ignoreWS) :
			m_ignoreWS{std::move(ignoreWS)} {}

		~EOFToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			skipWs();
			if (reader.GetChar() == 0) {
				return std::make_shared<SimpleValue>(ValueKind::WhiteSpace, reader.GetLocation(), reader);
			}
			return RewindWithError(reader.GetLocation(), reader, "Expected end of file");
		}
	};

	inline static std::shared_ptr<IToken> Eof(std::shared_ptr<IToken> ignoreWS) {
		return std::make_shared<EOFToken>(std::move(ignoreWS));
	}

	class DebugToken : public IToken {
		std::shared_ptr<IToken> m_token;
		std::string m_name;

	public:
		DebugToken(std::shared_ptr<IToken> token, std::string name) :
			m_token{std::move(token)},
			m_name{std::move(name)} {}

		~DebugToken() override = default;

		__forceinline std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
#ifdef FUNCC_DEBUG_TOKEN
			FUNCC_DEBUG_TOKEN_BREAK(m_name.c_str());
#endif
			return m_token->Consume(reader);
		}
	};

	inline static std::shared_ptr<IToken> Debug(std::shared_ptr<IToken> token, std::string name = "") {
		return std::make_shared<DebugToken>(std::move(token), std::move(name));
	}

	class ForwardDeclarationToken : public IToken {
		std::vector<std::shared_ptr<IToken>> m_token{};
		mutable int m_recursionDepth{0};

	public:
		class Replacement {
		public:
			Replacement(std::shared_ptr<IToken> target, std::vector<std::shared_ptr<IToken>> replacement) {
				std::dynamic_pointer_cast<ForwardDeclarationToken>(target)->m_token = std::move(replacement);
			}
		};

		~ForwardDeclarationToken() override = default;

		std::shared_ptr<ITokenValue> Consume(IReader& reader) const override {
			m_recursionDepth++;
			if (m_recursionDepth > 256) {
				return std::make_shared<ErrorValue>(
					Range{reader.GetLocation(), reader.GetLocation()},
					"Forward declaration recursion limit exceeded"
				);
			}

			std::shared_ptr<ITokenValue> result{};
			for (auto& token: m_token) {
				result = token->Consume(reader);
				if (result->HasValue()) {
					break;
				}
			}

			m_recursionDepth--;
			return result;
		}

		friend class Replacement;
	};

	inline static std::shared_ptr<IToken> ForwardDeclaration() {
		return std::make_shared<ForwardDeclarationToken>();
	}
}
