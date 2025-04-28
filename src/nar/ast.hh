#pragma once

#include <vector>

#include "../api.hh"
#include "../ast_common.hh"
#include "../ast_normalized.hh"

namespace funcc::nar {
	class IStatement {
	public:
		virtual ~IStatement() = default;
		virtual normalized::PStatement Normalize(Context const& ctx) const = 0;
	};

	class IType {
		Location m_location;

	public:
		IType(Location location) :
			m_location(location) {}

		virtual ~IType() = default;

		[[nodiscard]] Location const& GetLocation() const {
			return m_location;
		}

		virtual IType ApplyArgs(std::unordered_map<Identifier, IType> const& args, Location location) = 0;
	};

	class IExpression {
		Location m_location;

	public:
		IExpression(Location location) :
			m_location(location) {}

		[[nodiscard]] Location const& GetLocation() const {
			return m_location;
		}

		virtual ~IExpression() = default;
	};

	struct Import {
		Location location;
		QualifiedIdentifier module;
		Identifier alias;
		bool exposeAll;
		std::vector<Identifier> expose;
	};

	struct Alias {
		Location location;
		bool hidden;
		Identifier name;
		std::vector<Identifier> params;
		std::unique_ptr<IType> type;
	};

	enum class Associativity { Left = -1, None = 0, Right = 1 };

	struct InfixDefinition {
		Location location;
		bool hidden;
		InfixIdentifier name;
		Location nameLocation;
		Associativity associativity;
		int precedence;
		Alias alias;
	};

	struct IPattern {
	private:
		Location m_location;
		std::unique_ptr<IType> m_type;

	public:
		IPattern(Location location, std::unique_ptr<IType> type) :
			m_location(location),
			m_type(std::move(type)) {}

		virtual ~IPattern() = default;

		[[nodiscard]] Location const& GetLocation() const {
			return m_location;
		}

		[[nodiscard]] IType const& GetType() const {
			return *m_type;
		}
	};

	struct FunctionDefinition {
		Location location;
		bool hidden;
		Identifier name;
		Location nameLocation;
		std::vector<std::unique_ptr<IPattern>> params;
		std::unique_ptr<IType> type;
		std::unique_ptr<IExpression> body;
	};

	struct DataTypeConstructorValue {
		Location location;
		Identifier name;
		Location nameLocation;
		std::unique_ptr<IType> type;
	};

	struct DataTypeConstructor {
		Location location;
		bool hidden;
		Identifier name;
		Location nameLocation;
		std::vector<DataTypeConstructorValue> params;
	};

	struct DataType {
		Location location;
		bool hidden;
		Identifier name;
		Location nameLocation;
		std::vector<Identifier> params;
		std::vector<DataTypeConstructor> constructors;
	};

	struct File {
		QualifiedIdentifier module;
		Location location;
		std::vector<Import> imports;
		std::vector<Alias> aliases;
		std::vector<InfixDefinition> infixDefinitions;
		std::vector<FunctionDefinition> functionDefinitions;
		std::vector<DataType> dataTypes;
	};
}
