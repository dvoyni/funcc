#pragma once

#include <stdint.h>
#include <string_view>

#include "ast_common.hh"

namespace funcc {
	class IReader {
	public:
		virtual ~IReader() = default;

		[[nodiscard]] virtual uint32_t GetChar() const = 0;
		[[nodiscard]] virtual Location GetLocation() const = 0;
		[[nodiscard]] virtual std::string_view Sub(Range const& range) const = 0;

		virtual Location Push() = 0;
		virtual Location Pop() = 0;

		virtual bool Move() = 0;
	};

	class Utf8Reader : public IReader {
		std::string_view m_buffer;
		Location m_location;
		uint32_t m_currentChar;
		size_t m_currentLength;
		std::vector<Location> m_stack;

	public:
		Utf8Reader(std::string_view buffer) :
			m_buffer(std::move(buffer)),
			m_location{0, 1, 1},
			m_currentChar(0),
			m_currentLength(0) {
			Peek();
		}

		~Utf8Reader() override = default;

		[[nodiscard]] uint32_t GetChar() const override {
			return m_currentChar;
		}

		[[nodiscard]] Location GetLocation() const override {
			return m_location;
		}

		Location Push() override {
			m_stack.push_back(m_location);
			return m_location;
		}

		Location Pop() override {
			Location location = m_location;
			m_location = m_stack.back();
			m_stack.pop_back();
			Peek();
			return location;
		}

		[[nodiscard]] std::string_view Sub(Range const& range) const override {
			return m_buffer.substr(range.start.position, range.end.position - range.start.position);
		}

		bool Move() override {
			if (m_currentChar == '\n') {
				m_location.line++;
				m_location.column = 0;
			}
			m_location.column++;
			m_location.position += m_currentLength;
			Peek();

			return m_currentLength > 0;
		}

	private:
		void Peek() {
			m_currentChar = 0;
			m_currentLength = 0;

			if (m_location.position < m_buffer.size()) {
				// read next utf-8 character, position remains unchanged
				unsigned char c = m_buffer[m_location.position];
				size_t length = 0;
				if (c < 0x80) {
					length = 1;
				} else if ((m_currentChar & 0xE0) == 0xC0) {
					length = 2;
				} else if ((m_currentChar & 0xF0) == 0xE0) {
					length = 3;
				} else if ((m_currentChar & 0xF8) == 0xF0) {
					length = 4;
				}
				if (length > 0 && m_location.position + length <= m_buffer.size()) {
					m_currentLength = length;
					m_currentChar = c;
					for (size_t i = 1; i < m_currentLength; ++i) {
						m_currentChar = (m_currentChar << 6) | (m_buffer[m_location.position + i] & 0x3F);
					}
				}
			}
		}
	};
}
