#pragma once
#include <string>

// TODO: make it configurable in runtime
#ifndef TChar
#define TChar char
#endif

#ifndef TInt
#define TInt int
#endif

#ifndef TFloat
#define TFloat double
#endif

#ifndef TString
#define TString std::string
#endif

namespace funcc {
	using Identifier = std::string;
	using QualifiedIdentifier = std::string;
	using InfixIdentifier = std::string;
	using FullIdentifier = std::string;

	struct Location {
		size_t position;
		size_t line;
		size_t column;

		Location() = default;

		Location(size_t position, size_t line, size_t column) :
			position(position),
			line(line),
			column(column) {}

		Location(Location const& other) = default;
		Location& operator=(Location const& other) = default;

		bool operator<(Location const& other) const {
			return position < other.position;
		}
	};

	struct Range {
		Location start;
		Location end;

		Range() = default;

		explicit Range(Location start) :
			start{std::move(start)},
			end{std::move(start)} {}

		Range(Location start, Location end) :
			start{std::move(start)},
			end{std::move(end)} {}

		Range(Range const& other) = default;
		Range& operator=(Range const& other) = default;

		bool operator<(Range const& other) const {
			return start < other.start;
		}
	};

	class IConst {
	public:
		virtual ~IConst() = default;
	};

	class ConstChar final : public IConst {
		TChar m_value;

	public:
		ConstChar(TChar value) :
			m_value(std::move(value)) {}

		[[nodiscard]] TChar GetValue() const {
			return m_value;
		}
	};

	class ConstInt final : public IConst {
		TInt m_value;

	public:
		ConstInt(TInt value) :
			m_value(std::move(value)) {}

		~ConstInt() override = default;

		[[nodiscard]] TInt GetValue() const {
			return m_value;
		}
	};

	class ConstFloat final : public IConst {
		TFloat m_value;

	public:
		ConstFloat(TFloat value) :
			m_value(std::move(value)) {}

		~ConstFloat() override = default;

		[[nodiscard]] TFloat GetValue() const {
			return m_value;
		}
	};

	class ConstString final : public IConst {
		TString m_value;

	public:
		ConstString(TString value) :
			m_value(std::move(value)) {}

		~ConstString() override = default;

		[[nodiscard]] TString const& GetValue() const {
			return m_value;
		}
	};

	class ConstUnit final : public IConst {
	public:
		~ConstUnit() override = default;
	};
}
