#pragma once

#include "../_external.hh"
#include "../ast_common.hh"

namespace funcc::nar {
	using Identifier = std::string_view;
	using QualifiedIdentifier = std::string_view;
	using InfixIdentifier = std::string_view;
	using FullIdentifier = std::string_view;

	enum class Associativity { Left = -1, None = 0, Right = 1 };

	class IDeclaration {
		Range m_range;
		Identifier m_name;
		Range m_nameRange;
		bool m_hidden;

	public:
		IDeclaration(Range range, Identifier name, Range nameRange, bool hidden) :
			m_range{std::move(range)},
			m_name{std::move(name)},
			m_nameRange{std::move(nameRange)},
			m_hidden{hidden} {}

		virtual ~IDeclaration() = default;

		[[nodiscard]] Range const& GetRange() const {
			return m_range;
		}

		[[nodiscard]] Identifier const& GetName() const {
			return m_name;
		}

		[[nodiscard]] Range const& GetNameRange() const {
			return m_nameRange;
		}

		[[nodiscard]] bool IsHidden() const {
			return m_hidden;
		}
	};

	class IType {
		Range m_range;

	public:
		IType(Range range) :
			m_range{std::move(range)} {}

		IType(IType const&) = default;
		IType& operator=(IType const&) = default;

		virtual ~IType() = default;

		[[nodiscard]] Range const& GetRange() const {
			return m_range;
		}

		// virtual IType ApplyArgs(std::unordered_map<Identifier, IType> const& args, Range const& range) = 0;
	};

	class IExpression {
		Range m_range;

	public:
		IExpression(Range range) :
			m_range{std::move(range)} {}

		IExpression(IExpression const&) = default;
		IExpression& operator=(IExpression const&) = default;

		[[nodiscard]] Range const& GetRange() const {
			return m_range;
		}

		virtual ~IExpression() = default;
	};

	class IPattern {
	private:
		Range m_range;
		std::shared_ptr<IType> m_type;

	public:
		IPattern(Range range, std::shared_ptr<IType> type) :
			m_range{std::move(range)},
			m_type{std::move(type)} {}

		IPattern(IPattern const&) = default;
		IPattern& operator=(IPattern const&) = default;

		virtual ~IPattern() = default;

		[[nodiscard]] Range const& GetRange() const {
			return m_range;
		}

		[[nodiscard]] std::shared_ptr<IType> const& GetType() const {
			return m_type;
		}
	};

	struct DataConstructorParameter {
		Range range;
		Identifier name;
		Range nameRange;
		std::shared_ptr<IType> type;
	};

	struct DataConstructor {
		Range range;
		bool hidden;
		Identifier name;
		Range nameRange;
		std::vector<DataConstructorParameter> params;
	};

	struct Import {
		Range range;
		QualifiedIdentifier module;
		Identifier alias;
		bool exposeAll;
		std::vector<Identifier> expose;
	};

	struct File {
		QualifiedIdentifier module;
		Range moduleRange;
		std::vector<Import> imports;
		std::vector<std::shared_ptr<IDeclaration>> declarations;
	};
}
