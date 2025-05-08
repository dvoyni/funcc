#pragma once

#include <unordered_map>
#include <vector>

#include "../ast_common.hh"
#include "../ast_normalized.hh"

namespace funcc::nar {
	using Identifier = std::string_view;
	using QualifiedIdentifier = std::string_view;
	using InfixIdentifier = std::string_view;

	class IStatement {
	public:
		IStatement() = default;
		IStatement(IStatement const&) = default;
		IStatement& operator=(IStatement const&) = default;

		virtual ~IStatement() = default;
		// virtual normalized::PStatement Normalize(Context const& ctx) const = 0;
	};

	class IType {
		Range m_range;

	public:
		IType(Range range) :
			m_range(std::move(range)) {}

		IType(IType const&) = default;
		IType& operator=(IType const&) = default;

		virtual ~IType() = default;

		[[nodiscard]] Range const& GetRange() const {
			return m_range;
		}

		virtual IType ApplyArgs(std::unordered_map<Identifier, IType> const& args, Range const& range) = 0;
	};

	class IExpression {
		Range m_range;

	public:
		IExpression(Range range) :
			m_range(std::move(range)) {}

		IExpression(IExpression const&) = default;
		IExpression& operator=(IExpression const&) = default;

		[[nodiscard]] Range const& GetRange() const {
			return m_range;
		}

		virtual ~IExpression() = default;
	};

	struct Import {
		Range range;
		QualifiedIdentifier module;
		Identifier alias;
		bool exposeAll;
		std::vector<Identifier> expose;
	};

	struct Alias {
		Range range;
		bool hidden;
		Identifier name;
		std::vector<Identifier> params;
		std::shared_ptr<IType> type;
	};

	enum class Associativity { Left = -1, None = 0, Right = 1 };

	struct InfixDefinition {
		Range range;
		bool hidden;
		InfixIdentifier name;
		Range nameRange;
		Associativity associativity;
		int precedence;
		Alias alias;
	};

	class IPattern {
	private:
		Range m_range;
		std::shared_ptr<IType> m_type;

	public:
		IPattern(Range range, std::shared_ptr<IType> type) :
			m_range(std::move(range)),
			m_type(std::move(type)) {}

		IPattern(IPattern const&) = default;
		IPattern& operator=(IPattern const&) = default;

		virtual ~IPattern() = default;

		[[nodiscard]] Range const& GetRange() const {
			return m_range;
		}

		[[nodiscard]] IType const& GetType() const {
			return *m_type;
		}
	};

	struct FunctionDefinition {
		Range range;
		bool hidden;
		Identifier name;
		Range nameRange;
		std::vector<std::shared_ptr<IPattern>> params;
		std::shared_ptr<IType> type;
		std::shared_ptr<IExpression> body;
	};

	struct DataTypeConstructorValue {
		Range range;
		Identifier name;
		Range nameRange;
		std::shared_ptr<IType> type;
	};

	struct DataTypeConstructor {
		Range range;
		bool hidden;
		Identifier name;
		Range nameRange;
		std::vector<DataTypeConstructorValue> params;
	};

	struct DataType {
		Range range;
		bool hidden;
		Identifier name;
		Range nameRange;
		std::vector<Identifier> params;
		std::vector<DataTypeConstructor> constructors;
	};

	struct File {
		QualifiedIdentifier module;
		Range moduleRange;
		std::vector<Import> imports;
		std::vector<Alias> aliases;
		std::vector<InfixDefinition> infixDefinitions;
		std::vector<FunctionDefinition> functionDefinitions;
		std::vector<DataType> dataTypes;
	};
}
