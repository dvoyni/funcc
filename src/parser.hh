#pragma once

#include <cassert>
#include <format>
#include <functional>
#include <locale>
#include <string>
#include <tuple>
#include <vector>

#include "ast_common.hh"

namespace funcc {

	class Parser final {
		std::string m_input;
		size_t m_position;
		Location m_location;
		std::vector<std::tuple<size_t, Location>> m_stack;
		std::function<bool(char32_t)> m_isWhitespace;

	public:
		Parser(std::string const& input, std::function<bool(char32_t)> const& isWhitespace = &DefaultIsWhitespace) :
			m_input{input},
			m_position{0},
			m_location{1, 1},
			m_isWhitespace{isWhitespace} {}

		using ReadPredicate = std::function<bool(std::string_view acc, char32_t next)>;

		template<typename T>
		using MapFn = std::function<Result<T>(std::string_view token)>;

		template<typename T>
		Result<T> Read(ReadPredicate const& shouldAppend, MapFn<T> const& map, bool skipLeadingWhitespace = true) {
			Push();
			if (skipLeadingWhitespace) {
				SkipWhitespace();
			}
			std::string token = Read(shouldAppend);
			Result<T> result = map(token);
			if (!result) {
				Pop();
			}
			return result;
		}

		Result<std::string_view> Exact(
			std::string const& str,
			std::string error = "",
			bool skipLeadingWhitespace = true
		) {
			Push();
			if (skipLeadingWhitespace) {
				SkipWhitespace();
			}
			for (char c: str) {
				if (m_position >= m_input.size()) {
					Pop();
					return ErrorHere<std::string_view>(error);
				}
				if (m_input[m_position] != c) {
					Pop();
					return ErrorHere<std::string_view>(error);
				}
				StepForward();
			}
			return Result<std::string_view>(std::string_view{m_input.data() + m_position - str.size(), str.size()});
		}

		template<typename T>
		using ReadFn = std::function<Result<T>(Parser& parser)>;

		template<typename T>
		Result<T> OneOf(Result<T> furthestResult, ReadFn<T> const& head, ReadFn<T> const& rest...) {
			Result<T> result = head(*this);
		}

		template<typename T>
		Result<T> OneOf(ReadFn<T> const& readFns...) {
			Result<T> furthestResult{};
			for (ReadFn<T> const& readFn: readFns) {
				Push();
				Result<T> result = readFn(*this);
				if (result) {
					return result;
				} else {
					if (furthestResult < result) {
						furthestResult = result;
					}
				}
				Pop();
			}
			return furthestResult;
		}

		template<typename T, typename... Args>
		using BuildFn = std::function<Result<T>(Result<Args>...)>;

		template<typename T, typename... Args>
		Result<T> All(BuildFn<T, Args...> const& build, ReadFn<Args>... read) {
			Push();
			std::tuple results{read(*this)...};
			bool hasError = false;
			std::apply([&hasError](auto&&... args) { ((hasError |= args), ...); }, results);
			if (hasError) {
				Pop();
				return ErrorHere<T>(std::format("Expected all arguments here"));
			}
			return std::apply(build, results);
		}

		/*template<typename T, typename I>
		using ConsFn = std::function<Result<T>(std::vector<I> const&& items)>;

		template<typename T, typename I>
		Result<T> FewOf(
			std::string const& separator,
			ReadFn<I> const& readFn,
			JoinFn<T, I> const& joinFn,
			std::string const& begin = "",
			std::string const& end = "",
			bool skipWhitespace = true
		) {
			std::vector<I> items;
			Push();
			Result<std::string_view> beginResult =
				Exact(begin, std::format("Expected `{}` here", begin), skipWhitespace);
			if (!beginResult) {
				return beginResult;
			}

			bool expectingEnd = false;
			Result<std::string_view> separatorResult;

			while (true) {
				Result<std::string_view> endResult = Exact(end, "", skipWhitespace);
				if (endResult) {
					break;
				} else if (expectingEnd) {
					Result<T> error = ErrorHere<T>(std::format("Expected `{}` or `{}` here", separator, end));
					Pop();
					return error;
				}

				Result<I> itemResult = readFn(*this);
				if (!itemResult) {
					Pop();
					return itemResult;
				}
				items.push_back(itemResult.GetValue());

				separatorResult = Exact(separator, "", skipWhitespace);

				if (!separatorResult) {
					expectingEnd = true;
					break;
				}
			}

			if (separatorResult && items.empty()) {
				Result<T> error = ErrorHere<T>(std::format("Did not expect `{}` here", separator));
				Pop();
				return error;
			}

			Result<T> result = joinFn(std::move(items));
			if (!result) {
				Pop();
			}
			return result;
		}*/

		void Push() {
			m_stack.push_back(std::tuple(m_position, m_location));
		}

		void Pop() {
			auto [position, location] = m_stack.back();
			m_stack.pop_back();
			m_position = position;
			m_location = location;
		}

		template<typename T>
		Result<T> ErrorHere(std::string const& error) {
			return Result<T>(error, m_position, m_location);
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

		void AppendChar(std::string& to) {
			char32_t c;
			size_t length;
			if (Peek(c, length)) {
				for (size_t i = 0; i < length; ++i) {
					to += m_input[m_position];
					StepForward();
				}
			}
		}

		std::string Read(ReadPredicate const& shouldAppend) {
			std::string result;
			char32_t c;
			size_t length;
			while (Peek(c, length)) {
				if (shouldAppend(result, c)) {
					AppendChar(result);
				}
			}
			return result;
		}

		void StepForward() {
			if (m_input[m_position] == '\n') {
				m_location.line++;
				m_location.column = 0;
			}
			m_position++;
			m_location.column++;
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

		bool IsWhitespace(std::string_view acc, char32_t next) {
			return m_isWhitespace(next);
		}

		void SkipWhitespace() {
			Read([this](std::string_view acc, char32_t next) { return m_isWhitespace(next); });
		}

		inline static bool DefaultIsWhitespace(char32_t c) {
			return std::isspace(static_cast<char>(c), std::locale::classic());
		}
	};
}
