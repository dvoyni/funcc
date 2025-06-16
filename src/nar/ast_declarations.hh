#pragma once

#include "../_external.hh"
#include "ast_common.hh"

namespace funcc::nar {
	class Alias : public IDeclaration {
		std::shared_ptr<IType> m_type;
		std::vector<std::shared_ptr<IType>> m_typeParams;

	public:
		Alias(
			Range range,
			Identifier name,
			Range nameRange,
			bool hidden,
			std::shared_ptr<IType> type,
			std::vector<std::shared_ptr<IType>> typeParams
		) :
			IDeclaration{std::move(range), std::move(name), std::move(nameRange), hidden},
			m_type{std::move(type)},
			m_typeParams{std::move(typeParams)} {}

		~Alias() override = default;

		[[nodiscard]] std::vector<std::shared_ptr<IType>> const& GetTypeParams() const {
			return m_typeParams;
		}

		[[nodiscard]] IType const& GetType() const {
			return *m_type;
		}
	};

	class Infix : public IDeclaration {
		Associativity m_associativity;
		int64_t m_precedence;
		Identifier m_alias;

	public:
		Infix(
			Range range,
			InfixIdentifier name,
			Range nameRange,
			bool hidden,
			Associativity associativity,
			int64_t precedence,
			Identifier alias
		) :
			IDeclaration{std::move(range), std::move(static_cast<Identifier>(name)), std::move(nameRange), hidden},
			m_associativity{associativity},
			m_precedence{precedence},
			m_alias{std::move(alias)} {}

		~Infix() override = default;

		[[nodiscard]] Associativity GetAssociativity() const {
			return m_associativity;
		}

		[[nodiscard]] int64_t GetPrecedence() const {
			return m_precedence;
		}

		[[nodiscard]] Identifier const& GetAlias() const {
			return m_alias;
		}
	};

	class Function : public IDeclaration {
		std::vector<std::shared_ptr<IPattern>> m_params;
		std::shared_ptr<IType> m_type;
		std::shared_ptr<IExpression> m_body;

	public:
		Function(
			Range range,
			Identifier name,
			Range nameRange,
			bool hidden,
			std::vector<std::shared_ptr<IPattern>> params,
			std::shared_ptr<IType> type,
			std::shared_ptr<IExpression> body
		) :
			IDeclaration{std::move(range), std::move(name), std::move(nameRange), hidden},
			m_params{std::move(params)},
			m_type{std::move(type)},
			m_body{std::move(body)} {}

		~Function() override = default;

		[[nodiscard]] std::vector<std::shared_ptr<IPattern>> const& GetParams() const {
			return m_params;
		}

		[[nodiscard]] IType const& GetType() const {
			return *m_type;
		}

		[[nodiscard]] IExpression const& GetBody() const {
			return *m_body;
		}
	};

	class Data : public IDeclaration {
		std::vector<Identifier> m_typeParams;
		std::vector<DataConstructor> constructors;

	public:
		Data(
			Range range,
			Identifier name,
			Range nameRange,
			bool hidden,
			std::vector<Identifier> typeParams,
			std::vector<DataConstructor> constructors
		) :
			IDeclaration{std::move(range), std::move(name), std::move(nameRange), hidden},
			m_typeParams{std::move(typeParams)},
			constructors{std::move(constructors)} {}

		~Data() override = default;

		[[nodiscard]] std::vector<Identifier> const& GetTypeParams() const {
			return m_typeParams;
		}

		[[nodiscard]] std::vector<DataConstructor> const& GetConstructors() const {
			return constructors;
		}
	};
}
