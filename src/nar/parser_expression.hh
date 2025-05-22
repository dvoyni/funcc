#pragma once

#include "../_external.hh"
#include "ast_expressions.hh"
#include "parser_common.hh"

namespace funcc::nar {
	using namespace funcc::parser;

	class ExpressionParser {
		using C = CommonParser;

	public:
		using ExpressionValue = Value<std::shared_ptr<nar::IExpression>>;

		static std::shared_ptr<IToken> PExpression;

		inline static std::shared_ptr<IToken> PAccessor = Map(
			All(C::Tokens{Exact(C::SeqAccessor, C::PWS), C::PIdentifier}, C::PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionAccessor>(
						value->GetRange(),
						std::dynamic_pointer_cast<C::IdentifierValue>(value)->GetValue()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PAccess = Map(
			All(C::Tokens{PExpression, Repeat(Exact(C::SeqAccessor, C::PWS), PAccessor, C::PWS), C::PIdentifier}, C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionAccess>(
						value->GetRange(),
						std::dynamic_pointer_cast<ExpressionValue>(mv[0])->GetValue(),
						std::dynamic_pointer_cast<C::IdentifierValue>(mv[2])->GetValue(),
						mv[2]->GetRange()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PApply = Map(
			All(
				C::Tokens{
					PExpression,
					Some(
						PExpression,
						Exact(C::SeqFuncOpen, C::PWS),
						Exact(C::SeqFuncClose, C::PWS),
						Exact(C::SeqFuncSep, C::PWS),
						C::PWS
					)
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionApply>(
						value->GetRange(),
						std::dynamic_pointer_cast<ExpressionValue>(mv[0])->GetValue(),
						std::dynamic_pointer_cast<MultiValue>(mv[1])->Extract<std::shared_ptr<IExpression>>()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PBinOp = Map(
			All(
				C::Tokens{
					PExpression,
					C::PInfixIdentifier,  // TODO: add an option to call regular function as binary
										  // operator, like `2 \add 3` instead of `add(2, 3)`
					PExpression
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionBinOp>(
						value->GetRange(),
						std::dynamic_pointer_cast<ExpressionValue>(mv[0])->GetValue(),
						std::make_shared<ExpressionIfixVar>(
							mv[1]->GetRange(),
							std::dynamic_pointer_cast<C::InfixIdentifierValue>(mv[1])->GetValue()
						),
						std::dynamic_pointer_cast<ExpressionValue>(mv[2])->GetValue()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PConst =
			Map(C::PConst, [](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionConst>(
						value->GetRange(),
						std::dynamic_pointer_cast<C::ConstValue>(value)->GetValue()
					)
				);
			});

		inline static std::shared_ptr<IToken> PLetFunction = Map(All(C::Tokens{Exact(C::KwLet, C::PWS), C::}))
	};

	inline std::shared_ptr<IToken> ExpressionParser::PExpression = OneOf(CommonParser::Tokens{}, CommonParser::PWS);
}
