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
		int line;
		int column;

		Location(int line, int column) :
			line(line),
			column(column) {}
	};

	class IConst {
	public:
		virtual ~IConst() = default;
	};

	class ConstChar final : public IConst {
		TChar m_value;

	public:
		ConstChar(TChar value) :
			m_value(value) {}

		[[nodiscard]] TChar GetValue() const {
			return m_value;
		}
	};

	class ConstInt final : public IConst {
		TInt m_value;

	public:
		ConstInt(TInt value) :
			m_value(value) {}

		~ConstInt() override = default;

		[[nodiscard]] TInt GetValue() const {
			return m_value;
		}
	};

	class ConstFloat final : public IConst {
		TFloat m_value;

	public:
		ConstFloat(TFloat value) :
			m_value(value) {}

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
