#pragma once

#include <stdint.h>
#include <string_view>

#include "ast_common.hh"

namespace funcc {
	class IReader {
	public:
		virtual ~IReader() = default;

		[[nodiscard]] virtual uint32_t GeCurrentChar() const = 0;
		[[nodiscard]] virtual Location const GetLocation() const = 0;
		[[nodiscard]] virtual std::string_view Sub(Location const& start, Location const& end) const = 0;

		virtual void Move() = 0;
	};

	class Utf8Reader : public IReader {
		std::string_view m_buffer;
		Location m_location;
		uint32_t m_currentChar;
		size_t m_currentLength;

	public:
		Utf8Reader(std::string_view buffer) :
			m_buffer(buffer),
			m_location{0, 1, 1},
			m_currentChar(0),
			m_currentLength(0) {
			Peek();
		}

		~Utf8Reader() override = default;

		[[nodiscard]] uint32_t GeCurrentChar() const override {
			return m_currentChar;
		}

		[[nodiscard]] Location const GetLocation() const override {
			return m_location;
		}

		[[nodiscard]] std::string_view Sub(Location const& start, Location const& end) const override {
			return m_buffer.substr(start.position, end.position - start.position);
		}

		void Move() override {
			if (m_currentChar == '\n') {
				m_location.line++;
				m_location.column = 0;
			}
			m_location.column++;
			m_location.position += m_currentLength;
			Peek();
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
