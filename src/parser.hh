#pragma once

#include <algorithm>
#include <cassert>
#include <format>
#include <functional>
#include <locale>
#include <string>
#include <tuple>
#include <vector>

#include "ast_common.hh"

namespace funcc {
	class ParserFactory;

	template<typename T>
	using Parser = std::function<Result<T>(ParserFactory&)>;

	struct Token {
		std::string_view str;
		Range range;
	};

	class ParserFactory final {
		std::string m_input;
		size_t m_position;
		Location m_location;
		std::vector<std::tuple<size_t, Location>> m_stack;
		std::function<bool(char32_t)> m_isWhitespace;

	public:
		ParserFactory(
			std::string const& input,
			std::function<bool(char32_t)> const& isWhitespace = &DefaultIsWhitespace
		) :
			m_input{input},
			m_position{0},
			m_location{1, 1},
			m_isWhitespace{isWhitespace} {}

		template<typename T>
		using ResultConstructorFn = std::function<Result<T>(Token const& token)>;

		using PredicateFn = std::function<bool(std::string_view const& acc, char32_t next)>;

		template<typename T>
		Parser<T> Some(
			ResultConstructorFn<T> const& ctor,
			PredicateFn const& shouldAppend,
			bool skipLeadingWhitespace = true
		) {
			return [&ctor, &shouldAppend, skipLeadingWhitespace](ParserFactory& pf) {
				return pf.ReadSome<T>(ctor, shouldAppend, skipLeadingWhitespace);
			};
		}

		Parser<void> Some(PredicateFn const& shouldAppend, bool skipLeadingWhitespace = true) {
			return Some<void>(Discard(), shouldAppend, skipLeadingWhitespace);
		}

		template<typename T>
		Parser<T> Exact(
			ResultConstructorFn<T> const& ctor,
			std::string_view const& str,
			bool skipLeadingWhitespace = true
		) {
			return [&ctor, &str, skipLeadingWhitespace](ParserFactory& pf) {
				return pf.ReadExact<T>(ctor, str, skipLeadingWhitespace);
			};
		}

		Parser<void> Exact(std::string_view const& str, bool skipLeadingWhitespace = true) {
			return Exact<void>(Discard(), str, skipLeadingWhitespace);
		}

		Parser<void> ExactEof(bool skipLeadingWhitespace = true) {
			return [skipLeadingWhitespace](ParserFactory& pf) {
				pf.Push();
				if (skipLeadingWhitespace) {
					pf.SkipWhitespace();
				}
				if (pf.m_position >= pf.m_input.size()) {
					return Result<void>{};
				}
				pf.Pop();
				return pf.ErrorHere<void>("Expected EOF, but found more input.");
			};
		}

		template<typename T, typename... Parsers>
		Parser<T> OneOf(Parsers... parsers) {
			return [parsers...](ParserFactory& pf) { return pf.ReadOneOf<T, Parsers...>(parsers...); };
		}

		template<typename T, typename CombineFn, typename... Parsers>
		Parser<T> All(CombineFn const& combine, Parsers... parser) {
			return [&combine, parser...](ParserFactory& pf) {
				return pf.ReadAll<T, CombineFn, Parsers...>(combine, parser...);
			};
		}

		template<typename... Args>
		std::function<Result<void>(Result<Args>...)> Discard() {
			return [](Result<Args>...) { return Result<void>{}; };
		}

		std::function<Result<void>(Token const&)> Discard() {
			return [](Token const&) { return Result<void>{}; };
		}

		Parser<void> NoopSuccess() {
			return [](ParserFactory& pf) { return Result<void>{}; };
		}

		void Push() {
			m_stack.push_back(std::tuple(m_position, m_location));
		}

		void Pop() {
			std::tie(m_position, m_location) = m_stack.back();
		}

		template<typename T>
		Result<T> ErrorHere(std::string const& error) {
			return Result<T>(error, m_location);
		}

	private:
		char32_t PeekChar() {
			char32_t c;
			size_t length;
			if (Peek(c, length)) {
				return c;
			}
			return 0;
		}

		Token Read(PredicateFn const& shouldAppend) {
			Location start = m_location;
			std::string_view result;
			char32_t c;
			size_t length;
			while (Peek(c, length)) {
				if (shouldAppend(result, c)) {
					StepForward(length);
				}
			}
			return Token{result, Range(std::move(start), m_location)};
		}

		void StepForward(size_t length) {
			for (size_t i = 0; i < length; ++i) {
				if (m_input[m_position] == '\n') {
					m_location.line++;
					m_location.column = 0;
				}
				m_position++;
				m_location.column++;
			}
		}

		bool Peek(char32_t& outChar, size_t& outLength) {
			if (m_position >= m_input.size()) {
				return false;
			}
			// read next utf-8 character, position remains unchanged
			outChar = m_input[m_position];
			if (outChar < 0x80) {
				outLength = 1;
			} else if ((outChar & 0xE0) == 0xC0) {
				outLength = 2;
			} else if ((outChar & 0xF0) == 0xE0) {
				outLength = 3;
			} else if ((outChar & 0xF8) == 0xF0) {
				outLength = 4;
			} else {
				return false;  // invalid utf-8
			}
			if (m_position + outLength > m_input.size()) {
				return false;  // out of bounds
			}
			for (size_t i = 1; i < outLength; ++i) {
				outChar = (outChar << 6) | (m_input[m_position + i] & 0x3F);
			}
			return true;
		}

		bool IsWhitespace(std::string_view const& acc, char32_t next) {
			return m_isWhitespace(next);
		}

		void SkipWhitespace() {
			Read([this](std::string_view const& acc, char32_t next) { return m_isWhitespace(next); });
		}

		inline static bool DefaultIsWhitespace(char32_t c) {
			return std::isspace(static_cast<char>(c), std::locale::classic());
		}

		template<typename T>
		Result<T> ReadSome(
			ResultConstructorFn<T> const& ctor,
			PredicateFn const& shouldAppend,
			bool skipLeadingWhitespace = true
		) {
			Push();
			if (skipLeadingWhitespace) {
				SkipWhitespace();
			}
			Token token = Read(shouldAppend);
			Result<T> result = ctor(token);
			if (!result) {
				Pop();
			}
			return result;
		}

		template<typename T>
		Result<T> ReadExact(
			ResultConstructorFn<T> const& ctor,
			std::string_view const& str,
			bool skipLeadingWhitespace = true
		) {
			Location start = m_location;
			Push();
			if (skipLeadingWhitespace) {
				SkipWhitespace();
			}
			Result<T> result;
			for (char c: str) {
				if (m_position >= m_input.size() || m_input[m_position] != c) {
					Pop();
					return ErrorHere<T>(std::format("Expected '{}' here", str));
				}
				StepForward(1);
			}
			return ctor(Token{str, Range{std::move(start), m_location}});
		}

		template<typename T, typename... Parsers>
		Result<T> ReadOneOf(Parsers... parsers) {
			return ReadOneOfReq(Result<T>{}, parsers...);
		}

		template<typename T>
		Result<T> ReadOneOfReq(Result<T> furthestResult) {
			return furthestResult;
		}

		template<typename T, typename TParser, typename... Parsers>
		Result<T> ReadOneOfReq(Result<T> furthestResult, TParser head, Parsers... parsers) {
			Push();
			Result<T> result = head(*this);
			if (result) {
				return result;
			} else {
				Pop();
				return ReadOneOfReq(result > furthestResult ? result : furthestResult, parsers...);
			}
		}

		template<typename T, typename CombineFn, typename... Parsers>
		Result<T> ReadAll(CombineFn const& combine, Parsers... parsers) {
			Push();
			auto results = std::make_tuple(parsers(*this)...);
			Result<T> result;
			std::apply([&result](auto&&... r) { (((!r && result < r) ? (result = r) : r), ...); }, results);
			if (!result) {
				Pop();
			} else {
				result = std::apply(combine, results);
			}
			return result;
		}
	};
}
