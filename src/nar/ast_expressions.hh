#pragma once

#include "../_external.hh"
#include "ast_common.hh"

namespace funcc::nar {

	class ExpressionAccess final : public IExpression {
		std::shared_ptr<IExpression> m_record;
		Identifier m_fieldName;
		Range m_fieldNameRange;

	public:
		ExpressionAccess(Range range, std::shared_ptr<IExpression> record, Identifier fieldName, Range fieldNameRange) :
			IExpression(std::move(range)),
			m_record(std::move(record)),
			m_fieldName(std::move(fieldName)),
			m_fieldNameRange(std::move(fieldNameRange)) {}

		~ExpressionAccess() override = default;

		[[nodiscard]] IExpression const& GetRecord() const {
			return *m_record;
		}

		[[nodiscard]] Identifier const& GetFieldName() const {
			return m_fieldName;
		}

		[[nodiscard]] Range const& GetFieldNameRange() const {
			return m_fieldNameRange;
		}
	};

	class ExpressionAccessor final : public IExpression {
		Identifier m_fieldName;

	public:
		ExpressionAccessor(Range range, Identifier fieldName) :
			IExpression(std::move(range)),
			m_fieldName(std::move(fieldName)) {}

		~ExpressionAccessor() override = default;

		[[nodiscard]] Identifier const& GetFieldName() const {
			return m_fieldName;
		}
	};

	class ExpressionApply final : public IExpression {
		std::shared_ptr<IExpression> m_function;
		std::vector<std::shared_ptr<IExpression>> m_args;

	public:
		ExpressionApply(
			Range range,
			std::shared_ptr<IExpression> function,
			std::vector<std::shared_ptr<IExpression>>&& args
		) :
			IExpression(std::move(range)),
			m_function(std::move(function)),
			m_args(std::move(args)) {}

		~ExpressionApply() override = default;

		[[nodiscard]] IExpression const& GetFunction() const {
			return *m_function;
		}

		[[nodiscard]] std::vector<std::shared_ptr<IExpression>> const& GetArgs() const {
			return m_args;
		}
	};

	class ExpressionBinOp final : public IExpression {
	private:
		std::shared_ptr<IExpression> m_left;
		std::shared_ptr<IExpression> m_op;
		std::shared_ptr<IExpression> m_right;

	public:
		ExpressionBinOp(
			Range range,
			std::shared_ptr<IExpression> left,
			std::shared_ptr<IExpression> op,
			std::shared_ptr<IExpression> right
		) :
			IExpression(std::move(range)),
			m_left(std::move(left)),
			m_op(std::move(op)),
			m_right(std::move(right)) {}

		~ExpressionBinOp() override = default;

		[[nodiscard]] IExpression const& GetLeft() const {
			return *m_left;
		}

		[[nodiscard]] IExpression const& GetOp() const {
			return *m_op;
		}

		[[nodiscard]] IExpression const& GetRight() const {
			return *m_right;
		}
	};

	class ExpressionCall final : public IExpression {
		FullIdentifier m_name;
		Range m_nameRange;
		std::vector<std::shared_ptr<IExpression>> m_args;

	public:
		ExpressionCall(
			Range range,
			FullIdentifier name,
			Range nameRange,
			std::vector<std::shared_ptr<IExpression>>&& args
		) :
			IExpression(std::move(range)),
			m_name(std::move(name)),
			m_nameRange(std::move(nameRange)),
			m_args(std::move(args)) {}

		~ExpressionCall() override = default;

		[[nodiscard]] FullIdentifier const& GetName() const {
			return m_name;
		}

		[[nodiscard]] Range const& GetNameRange() const {
			return m_nameRange;
		}

		[[nodiscard]] std::vector<std::shared_ptr<IExpression>> const& GetArgs() const {
			return m_args;
		}
	};

	class ExpressionConst final : public IExpression {
		std::shared_ptr<IConst> m_value;

	public:
		ExpressionConst(Range range, std::shared_ptr<IConst> value) :
			IExpression(std::move(range)),
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
		Range m_nameRange;
		std::vector<std::shared_ptr<IExpression>> m_args;

	public:
		ExpressionConstructor(
			Range range,
			QualifiedIdentifier module,
			Identifier data,
			Identifier option,
			Range nameRange,
			std::vector<std::shared_ptr<IExpression>>&& args
		) :
			IExpression(std::move(range)),
			m_module(std::move(module)),
			m_data(std::move(data)),
			m_option(std::move(option)),
			m_nameRange(std::move(nameRange)),
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

		[[nodiscard]] Range const& GetNameRange() const {
			return m_nameRange;
		}

		[[nodiscard]] std::vector<std::shared_ptr<IExpression>> const& GetArgs() const {
			return m_args;
		}
	};

	class ExpressionLetFunction final : public IExpression {
		Identifier m_name;
		Range m_nameRange;
		std::vector<std::shared_ptr<IPattern>> m_params;
		std::shared_ptr<IExpression> m_body;
		std::shared_ptr<IType> m_type;
		std::shared_ptr<IExpression> m_nested;

	public:
		ExpressionLetFunction(
			Range range,
			Identifier name,
			Range nameRange,
			std::vector<std::shared_ptr<IPattern>>&& params,
			std::shared_ptr<IExpression> body,
			std::shared_ptr<IType> type,
			std::shared_ptr<IExpression> nested
		) :
			IExpression(std::move(range)),
			m_name(std::move(name)),
			m_nameRange(std::move(nameRange)),
			m_params(std::move(params)),
			m_body(std::move(body)),
			m_type(std::move(type)),
			m_nested(std::move(nested)) {}

		~ExpressionLetFunction() override = default;

		[[nodiscard]] Identifier const& GetName() const {
			return m_name;
		}

		[[nodiscard]] Range const& GetNameRange() const {
			return m_nameRange;
		}

		[[nodiscard]] std::vector<std::shared_ptr<IPattern>> const& GetParams() const {
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
		std::shared_ptr<IExpression> m_condition;
		std::shared_ptr<IExpression> m_trueBranch;
		std::shared_ptr<IExpression> m_falseBranch;

	public:
		ExpressionIf(
			Range range,
			std::shared_ptr<IExpression> condition,
			std::shared_ptr<IExpression> trueBranch,
			std::shared_ptr<IExpression> falseBranch
		) :
			IExpression(std::move(range)),
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
		ExpressionIfixVar(Range range, InfixIdentifier infix) :
			IExpression(std::move(range)),
			m_infix(std::move(infix)) {}

		~ExpressionIfixVar() override = default;

		[[nodiscard]] InfixIdentifier const& GetInfix() const {
			return m_infix;
		}
	};

	class ExpressionLambda final : public IExpression {
		std::vector<std::shared_ptr<IPattern>> m_params;
		std::shared_ptr<IExpression> m_body;
		std::shared_ptr<IType> m_returnType;

	public:
		ExpressionLambda(
			Range range,
			std::vector<std::shared_ptr<IPattern>>&& params,
			std::shared_ptr<IExpression> body,
			std::shared_ptr<IType> returnType
		) :
			IExpression(std::move(range)),
			m_params(std::move(params)),
			m_body(std::move(body)),
			m_returnType(std::move(returnType)) {}

		~ExpressionLambda() override = default;

		[[nodiscard]] std::vector<std::shared_ptr<IPattern>> const& GetParams() const {
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
		std::shared_ptr<IPattern> m_pattern;
		std::shared_ptr<IExpression> m_value;
		std::shared_ptr<IExpression> m_nested;

	public:
		ExpressionLetVar(
			Range range,
			std::shared_ptr<IPattern> pattern,
			std::shared_ptr<IExpression> value,
			std::shared_ptr<IExpression> nested
		) :
			IExpression(std::move(range)),
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
		std::vector<std::shared_ptr<IExpression>> m_expressions;

	public:
		ExpressionList(Range range, std::vector<std::shared_ptr<IExpression>>&& expressions) :
			IExpression(std::move(range)),
			m_expressions(std::move(expressions)) {}

		~ExpressionList() override = default;

		[[nodiscard]] std::vector<std::shared_ptr<IExpression>> const& GetExpressions() const {
			return m_expressions;
		}
	};

	class ExpressionNegate final : public IExpression {
		std::shared_ptr<IExpression> m_expression;

	public:
		ExpressionNegate(Range range, std::shared_ptr<IExpression> expression) :
			IExpression(std::move(range)),
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
			Range nameRange;
			std::shared_ptr<IExpression> value;
		};

	private:
		std::vector<Field> m_fields;

	public:
		ExpressionRecord(Range range, std::vector<Field>&& fields) :
			IExpression(std::move(range)),
			m_fields(std::move(fields)) {}

		~ExpressionRecord() override = default;

		[[nodiscard]] std::vector<Field> const& GetFields() const {
			return m_fields;
		}
	};

	class ExpressionSelector final : public IExpression {
	public:
		struct Case {
			Range range;
			std::shared_ptr<IPattern> pattern;
			std::shared_ptr<IExpression> expression;
		};

	private:
		std::shared_ptr<IExpression> m_condition;

		std::vector<Case> m_cases;

	public:
		ExpressionSelector(Range range, std::shared_ptr<IExpression> condition, std::vector<Case>&& cases) :
			IExpression(std::move(range)),
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
		std::vector<std::shared_ptr<IExpression>> m_expressions;

	public:
		ExpressionTuple(Range range, std::vector<std::shared_ptr<IExpression>>&& expressions) :
			IExpression(std::move(range)),
			m_expressions(std::move(expressions)) {}

		~ExpressionTuple() override = default;

		[[nodiscard]] std::vector<std::shared_ptr<IExpression>> const& GetExpressions() const {
			return m_expressions;
		}
	};

	class ExpressionUpdate final : public IExpression {
	public:
		struct Field {
			Range range;
			Identifier name;
			Range nameRange;
			std::shared_ptr<IExpression> value;
		};

	private:
		std::shared_ptr<IExpression> m_record;
		std::vector<Field> m_fields;

	public:
		ExpressionUpdate(Range range, std::shared_ptr<IExpression> record, std::vector<Field>&& fields) :
			IExpression(std::move(range)),
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
		ExpressionVar(Range range, QualifiedIdentifier name) :
			IExpression(std::move(range)),
			m_name(std::move(name)) {}

		~ExpressionVar() override = default;

		[[nodiscard]] QualifiedIdentifier const& GetName() const {
			return m_name;
		}
	};
}
