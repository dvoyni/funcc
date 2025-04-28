#pragma once

#include <vector>

#include "ast.hh"

namespace funcc::nar {

	class ExpressionAccess final : public IExpression {
		std::unique_ptr<IExpression> m_record;
		Identifier m_fieldName;
		Location m_fieldNameLocation;

	public:
		ExpressionAccess(
			Location location,
			std::unique_ptr<IExpression> record,
			Identifier fieldName,
			Location fieldNameLocation
		) :
			IExpression(location),
			m_record(std::move(record)),
			m_fieldName(std::move(fieldName)),
			m_fieldNameLocation(fieldNameLocation) {}

		~ExpressionAccess() override = default;

		[[nodiscard]] IExpression const& GetRecord() const {
			return *m_record;
		}

		[[nodiscard]] Identifier const& GetFieldName() const {
			return m_fieldName;
		}

		[[nodiscard]] Location const& GetFieldNameLocation() const {
			return m_fieldNameLocation;
		}
	};

	class ExpressionAccessor final : public IExpression {
		std::unique_ptr<IExpression> m_fieldName;

	public:
		ExpressionAccessor(Location location, std::unique_ptr<IExpression> fieldName) :
			IExpression(location),
			m_fieldName(std::move(fieldName)) {}

		~ExpressionAccessor() override = default;

		[[nodiscard]] IExpression const& GetFieldName() const {
			return *m_fieldName;
		}
	};

	class ExpressionApply final : public IExpression {
		std::unique_ptr<IExpression> m_function;
		std::vector<std::unique_ptr<IExpression>> m_args;

	public:
		ExpressionApply(
			Location location,
			std::unique_ptr<IExpression> function,
			std::vector<std::unique_ptr<IExpression>>&& args
		) :
			IExpression(location),
			m_function(std::move(function)),
			m_args(std::move(args)) {}

		~ExpressionApply() override = default;

		[[nodiscard]] IExpression const& GetFunction() const {
			return *m_function;
		}

		[[nodiscard]] std::vector<std::unique_ptr<IExpression>> const& GetArgs() const {
			return m_args;
		}
	};

	class ExpressionBinOp final : public IExpression {
	public:
		struct Operand {
			std::unique_ptr<IExpression> operand;
			Identifier infix;
		};

	private:
		std::vector<Operand> m_operands;
		bool m_inParentheses;

	public:
		ExpressionBinOp(Location location, std::vector<Operand>&& operands, bool inParentheses) :
			IExpression(location),
			m_operands(std::move(operands)),
			m_inParentheses(inParentheses) {}

		~ExpressionBinOp() override = default;

		[[nodiscard]] std::vector<Operand> const& GetOperands() const {
			return m_operands;
		}

		[[nodiscard]] bool IsInParentheses() const {
			return m_inParentheses;
		}
	};

	class ExpressionCall final : public IExpression {
		FullIdentifier m_name;
		Location m_nameLocation;
		std::vector<std::unique_ptr<IExpression>> m_args;

	public:
		ExpressionCall(
			Location location,
			FullIdentifier name,
			Location nameLocation,
			std::vector<std::unique_ptr<IExpression>>&& args
		) :
			IExpression(location),
			m_name(std::move(name)),
			m_nameLocation(nameLocation),
			m_args(std::move(args)) {}

		~ExpressionCall() override = default;

		[[nodiscard]] FullIdentifier const& GetName() const {
			return m_name;
		}

		[[nodiscard]] Location const& GetNameLocation() const {
			return m_nameLocation;
		}

		[[nodiscard]] std::vector<std::unique_ptr<IExpression>> const& GetArgs() const {
			return m_args;
		}
	};

	class ExpressionConst final : public IExpression {
		std::shared_ptr<IConst> m_value;

	public:
		ExpressionConst(Location location, std::shared_ptr<IConst> value) :
			IExpression(location),
			m_value(std::move(value)) {}

		~ExpressionConst() override = default;

		[[nodiscard]] IConst const& GetValue() const {
			return *m_value;
		}
	};

	class ExpressionConstructor final : public IExpression {
		QualifiedIdentifier m_module;
		Identifier m_data;
		Identifier m_option;
		Location m_nameLocation;
		std::vector<std::unique_ptr<IExpression>> m_args;

	public:
		ExpressionConstructor(
			Location location,
			QualifiedIdentifier module,
			Identifier data,
			Identifier option,
			Location nameLocation,
			std::vector<std::unique_ptr<IExpression>>&& args
		) :
			IExpression(location),
			m_module(std::move(module)),
			m_data(std::move(data)),
			m_option(std::move(option)),
			m_nameLocation(nameLocation),
			m_args(std::move(args)) {}

		~ExpressionConstructor() override = default;

		[[nodiscard]] QualifiedIdentifier const& GetModule() const {
			return m_module;
		}

		[[nodiscard]] Identifier const& GetData() const {
			return m_data;
		}

		[[nodiscard]] Identifier const& GetOption() const {
			return m_option;
		}

		[[nodiscard]] Location const& GetNameLocation() const {
			return m_nameLocation;
		}

		[[nodiscard]] std::vector<std::unique_ptr<IExpression>> const& GetArgs() const {
			return m_args;
		}
	};

	class ExpressionLetFunction final : public IExpression {
		Identifier m_name;
		Location m_nameLocation;
		std::vector<std::unique_ptr<IPattern>> m_params;
		std::unique_ptr<IExpression> m_body;
		std::unique_ptr<IType> m_type;
		std::unique_ptr<IExpression> m_nested;

	public:
		ExpressionLetFunction(
			Location location,
			Identifier name,
			Location nameLocation,
			std::vector<std::unique_ptr<IPattern>>&& params,
			std::unique_ptr<IExpression> body,
			std::unique_ptr<IType> type,
			std::unique_ptr<IExpression> nested
		) :
			IExpression(location),
			m_name(std::move(name)),
			m_nameLocation(nameLocation),
			m_params(std::move(params)),
			m_body(std::move(body)),
			m_type(std::move(type)),
			m_nested(std::move(nested)) {}

		~ExpressionLetFunction() override = default;

		[[nodiscard]] Identifier const& GetName() const {
			return m_name;
		}

		[[nodiscard]] Location const& GetNameLocation() const {
			return m_nameLocation;
		}

		[[nodiscard]] std::vector<std::unique_ptr<IPattern>> const& GetParams() const {
			return m_params;
		}

		[[nodiscard]] IExpression const& GetBody() const {
			return *m_body;
		}

		[[nodiscard]] IType const& GetType() const {
			return *m_type;
		}

		[[nodiscard]] IExpression const& GetNested() const {
			return *m_nested;
		}
	};

	class ExpressionIf final : public IExpression {
		std::unique_ptr<IExpression> m_condition;
		std::unique_ptr<IExpression> m_trueBranch;
		std::unique_ptr<IExpression> m_falseBranch;

	public:
		ExpressionIf(
			Location location,
			std::unique_ptr<IExpression> condition,
			std::unique_ptr<IExpression> trueBranch,
			std::unique_ptr<IExpression> falseBranch
		) :
			IExpression(location),
			m_condition(std::move(condition)),
			m_trueBranch(std::move(trueBranch)),
			m_falseBranch(std::move(falseBranch)) {}

		~ExpressionIf() override = default;

		[[nodiscard]] IExpression const& GetCondition() const {
			return *m_condition;
		}

		[[nodiscard]] IExpression const& GetTrueBranch() const {
			return *m_trueBranch;
		}

		[[nodiscard]] IExpression const& GetFalseBranch() const {
			return *m_falseBranch;
		}
	};

	class ExpressionIfixVar final : public IExpression {
		InfixIdentifier m_infix;

	public:
		ExpressionIfixVar(Location location, InfixIdentifier infix) :
			IExpression(location),
			m_infix(std::move(infix)) {}

		~ExpressionIfixVar() override = default;

		[[nodiscard]] InfixIdentifier const& GetInfix() const {
			return m_infix;
		}
	};

	class ExpressionLambda final : public IExpression {
		std::vector<std::unique_ptr<IPattern>> m_params;
		std::unique_ptr<IExpression> m_body;
		std::unique_ptr<IType> m_returnType;

	public:
		ExpressionLambda(
			Location location,
			std::vector<std::unique_ptr<IPattern>>&& params,
			std::unique_ptr<IExpression> body,
			std::unique_ptr<IType> returnType
		) :
			IExpression(location),
			m_params(std::move(params)),
			m_body(std::move(body)),
			m_returnType(std::move(returnType)) {}

		~ExpressionLambda() override = default;

		[[nodiscard]] std::vector<std::unique_ptr<IPattern>> const& GetParams() const {
			return m_params;
		}

		[[nodiscard]] IExpression const& GetBody() const {
			return *m_body;
		}

		[[nodiscard]] IType const& GetReturnType() const {
			return *m_returnType;
		}
	};

	class ExpressionLetVar final : public IExpression {
		std::unique_ptr<IPattern> m_pattern;
		std::unique_ptr<IExpression> m_value;
		std::unique_ptr<IExpression> m_nested;

	public:
		ExpressionLetVar(
			Location location,
			std::unique_ptr<IPattern> pattern,
			std::unique_ptr<IExpression> value,
			std::unique_ptr<IExpression> nested
		) :
			IExpression(location),
			m_pattern(std::move(pattern)),
			m_value(std::move(value)),
			m_nested(std::move(nested)) {}

		~ExpressionLetVar() override = default;

		[[nodiscard]] IPattern const& GetPattern() const {
			return *m_pattern;
		}

		[[nodiscard]] IExpression const& GetValue() const {
			return *m_value;
		}

		[[nodiscard]] IExpression const& GetNested() const {
			return *m_nested;
		}
	};

	class ExpressionList final : public IExpression {
		std::vector<std::unique_ptr<IExpression>> m_expressions;

	public:
		ExpressionList(Location location, std::vector<std::unique_ptr<IExpression>>&& expressions) :
			IExpression(location),
			m_expressions(std::move(expressions)) {}

		~ExpressionList() override = default;

		[[nodiscard]] std::vector<std::unique_ptr<IExpression>> const& GetExpressions() const {
			return m_expressions;
		}
	};

	class ExpressionNegate final : public IExpression {
		std::unique_ptr<IExpression> m_expression;

	public:
		ExpressionNegate(Location location, std::unique_ptr<IExpression> expression) :
			IExpression(location),
			m_expression(std::move(expression)) {}

		~ExpressionNegate() override = default;

		[[nodiscard]] IExpression const& GetExpression() const {
			return *m_expression;
		}
	};

	class ExpressionRecord final : public IExpression {
	public:
		struct Field {
			Identifier name;
			Location nameLocation;
			std::unique_ptr<IExpression> value;
		};

	private:
		std::vector<Field> m_fields;

	public:
		ExpressionRecord(Location location, std::vector<Field>&& fields) :
			IExpression(location),
			m_fields(std::move(fields)) {}

		~ExpressionRecord() override = default;

		[[nodiscard]] std::vector<Field> const& GetFields() const {
			return m_fields;
		}
	};

	class ExpressionSelector final : public IExpression {
	public:
		struct Case {
			Location location;
			std::unique_ptr<IPattern> pattern;
			std::unique_ptr<IExpression> expression;
		};

	private:
		std::unique_ptr<IExpression> m_condition;

		std::vector<Case> m_cases;

	public:
		ExpressionSelector(Location location, std::unique_ptr<IExpression> condition, std::vector<Case>&& cases) :
			IExpression(location),
			m_condition(std::move(condition)),
			m_cases(std::move(cases)) {}

		~ExpressionSelector() override = default;

		[[nodiscard]] IExpression const& GetCondition() const {
			return *m_condition;
		}

		[[nodiscard]] std::vector<Case> const& GetCases() const {
			return m_cases;
		}
	};

	class ExpressionTuple final : public IExpression {
		std::vector<std::unique_ptr<IExpression>> m_expressions;

	public:
		ExpressionTuple(Location location, std::vector<std::unique_ptr<IExpression>>&& expressions) :
			IExpression(location),
			m_expressions(std::move(expressions)) {}

		~ExpressionTuple() override = default;

		[[nodiscard]] std::vector<std::unique_ptr<IExpression>> const& GetExpressions() const {
			return m_expressions;
		}
	};

	class ExpressionUpdate final : public IExpression {
	public:
		struct Field {
			Location location;
			Identifier name;
			Location nameLocation;
			std::unique_ptr<IExpression> value;
		};

	private:
		std::unique_ptr<IExpression> m_record;
		std::vector<Field> m_fields;

	public:
		ExpressionUpdate(Location location, std::unique_ptr<IExpression> record, std::vector<Field>&& fields) :
			IExpression(location),
			m_record(std::move(record)),
			m_fields(std::move(fields)) {}

		~ExpressionUpdate() override = default;

		[[nodiscard]] IExpression const& GetRecord() const {
			return *m_record;
		}

		[[nodiscard]] std::vector<Field> const& GetFields() const {
			return m_fields;
		}
	};

	class ExpressionVar final : public IExpression {
		QualifiedIdentifier m_name;

	public:
		ExpressionVar(Location location, QualifiedIdentifier name) :
			IExpression(location),
			m_name(std::move(name)) {}

		~ExpressionVar() override = default;

		[[nodiscard]] QualifiedIdentifier const& GetName() const {
			return m_name;
		}
	};
}
