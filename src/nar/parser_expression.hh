#pragma once

#include "../_external.hh"
#include "ast_expressions.hh"
#include "parser_common.hh"
#include "parser_pattern.hh"
#include "parser_type.hh"

namespace funcc::nar {
	using namespace funcc::parser;

	class ExpressionParser {
		using C = CommonParser;
		using P = PatternParser;
		using T = TypeParser;

	public:
		using ExpressionValue = Value<std::shared_ptr<nar::IExpression>>;

		inline static std::shared_ptr<IToken> PExpression = ForwardDeclaration();
		inline static std::shared_ptr<IToken> PLet = ForwardDeclaration();

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
					C::PInfixIdentifier,  // TODO: add an option to call regular function as infix
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
						std::make_shared<ExpressionInfixVar>(
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

		inline static std::shared_ptr<IToken> PIf = Map(
			All(
				C::Tokens{
					Exact(C::KwIf, C::PWS),
					PExpression,
					Exact(C::KwThen, C::PWS),
					PExpression,
					Exact(C::KwElse, C::PWS),
					PExpression
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionIf>(
						value->GetRange(),
						std::dynamic_pointer_cast<ExpressionValue>(mv[1])->GetValue(),
						std::dynamic_pointer_cast<ExpressionValue>(mv[3])->GetValue(),
						std::dynamic_pointer_cast<ExpressionValue>(mv[5])->GetValue()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PInfix = Map(
			C::PWrappedInfixIdentifier,
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionInfixVar>(
						value->GetRange(),
						std::dynamic_pointer_cast<C::InfixIdentifierValue>(value)->GetValue()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PLambda = Map(
			All(
				C::Tokens{
					Exact(C::SeqLambdaSignature, C::PWS),
					Some(P::PPattern, nullptr, Exact(C::SeqFuncClose, C::PWS), Exact(C::SeqFuncSep, C::PWS), C::PWS),
					Optional(T::PTypeAnnotation),
					Exact(C::SeqLambdaBind, C::PWS),
					PExpression,
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionLambda>(
						value->GetRange(),
						std::dynamic_pointer_cast<MultiValue>(mv[1])->Extract<std::shared_ptr<IPattern>>(),
						mv[2]->IsSkipped() ? nullptr : std::dynamic_pointer_cast<T::TypeValue>(mv[2])->GetValue(),
						std::dynamic_pointer_cast<ExpressionValue>(mv[4])->GetValue()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PLetFunction = Map(
			All(
				C::Tokens{
					Exact(C::KwLet, C::PWS),
					P::PFunctionSignature,
					Exact(C::SeqFunctionBind, C::PWS),
					PExpression,
					OneOf(
						C::Tokens{
							Map(All(C::Tokens{Exact(C::KwIn, C::PWS), PExpression}, C::PWS),
								[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
									return std::dynamic_pointer_cast<MultiValue>(value)->GetValues()[1];
								}),
							PLet,
						},
						C::PWS
					),
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				P::FunctionSignature signature = std::dynamic_pointer_cast<P::FunctionSignatureValue>(mv[1])->GetValue(
				);
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionLetFunction>(
						value->GetRange(),
						signature.name,
						signature.nameRange,
						std::move(signature.params),
						signature.returnType,
						std::dynamic_pointer_cast<ExpressionValue>(mv[3])->GetValue(),
						std::dynamic_pointer_cast<ExpressionValue>(mv[4])->GetValue()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PLetValue = Map(
			All(
				C::Tokens{
					Exact(C::KwLet, C::PWS),
					P::PPattern,
					Exact(C::SeqFunctionBind, C::PWS),
					PExpression,
					OneOf(
						C::Tokens{
							Map(All(C::Tokens{Exact(C::KwIn, C::PWS), PExpression}, C::PWS),
								[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
									return std::dynamic_pointer_cast<MultiValue>(value)->GetValues()[1];
								}),
							PLet,
						},
						C::PWS
					),
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionLetVar>(
						value->GetRange(),
						std::dynamic_pointer_cast<P::PatternValue>(mv[1])->GetValue(),
						std::dynamic_pointer_cast<ExpressionValue>(mv[3])->GetValue(),
						std::dynamic_pointer_cast<ExpressionValue>(mv[4])->GetValue()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PList = Map(
			Debug(Some(
				PExpression,
				Exact(C::SeqListOpen, C::PWS),
				Exact(C::SeqListClose, C::PWS),
				Exact(C::SeqListSep, C::PWS),
				C::PWS,
				nullptr,  // firstItem
				true  // allowEmpty
			)),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionList>(
						value->GetRange(),
						std::dynamic_pointer_cast<MultiValue>(mv[0])->Extract<std::shared_ptr<IExpression>>()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PNegate = Map(
			All(C::Tokens{Exact(C::SeqNegate, C::PWS), PExpression}, C::PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionNegate>(
						value->GetRange(),
						std::dynamic_pointer_cast<ExpressionValue>(value)->GetValue()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PRecord = Map(
			Some(
				All(C::Tokens{C::PIdentifier, Exact(C::SeqRecordBind, C::PWS), PExpression}, C::PWS),
				Exact(C::SeqRecordOpen, C::PWS),
				Exact(C::SeqRecordClose, C::PWS),
				Exact(C::SeqRecordSep, C::PWS),
				C::PWS,
				nullptr,  // firstItem
				true  // allowEmpty
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionRecord>(
						value->GetRange(),
						std::dynamic_pointer_cast<MultiValue>(value)->Extract<ExpressionRecord::Field>(
							[](std::shared_ptr<ITokenValue> const& fieldValue) -> ExpressionRecord::Field {
								C::Values mv = std::dynamic_pointer_cast<MultiValue>(fieldValue)->GetValues();
								return ExpressionRecord::Field{
									.range = fieldValue->GetRange(),
									.name = std::dynamic_pointer_cast<C::IdentifierValue>(mv[0])->GetValue(),
									.nameRange = mv[0]->GetRange(),
									.value = std::dynamic_pointer_cast<ExpressionValue>(mv[2])->GetValue()
								};
							}
						)
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PSelect = Map(
			All(
				C::Tokens{
					Exact(C::KwSelect, C::PWS),
					PExpression,
					Repeat(
						Exact(C::KwCase, C::PWS),
						All(C::Tokens{Exact(C::KwCase, C::PWS), P::PPattern, Exact(C::SeqCaseBind, C::PWS), PExpression},
							C::PWS),
						C::PWS
					),
					Exact(C::KwEnd, C::PWS)
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionSelect>(
						value->GetRange(),
						std::dynamic_pointer_cast<ExpressionValue>(mv[1])->GetValue(),
						std::dynamic_pointer_cast<MultiValue>(mv[2])->Extract<ExpressionSelect::Case>(
							[](std::shared_ptr<ITokenValue> const& caseValue) -> ExpressionSelect::Case {
								C::Values mv = std::dynamic_pointer_cast<MultiValue>(caseValue)->GetValues();
								return ExpressionSelect::Case{
									.range = caseValue->GetRange(),
									.pattern = std::dynamic_pointer_cast<P::PatternValue>(mv[1])->GetValue(),
									.expression = std::dynamic_pointer_cast<ExpressionValue>(mv[3])->GetValue()
								};
							}
						)
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PTuple = Map(
			Some(
				PExpression,
				Exact(C::SeqTupleOpen, C::PWS),
				Exact(C::SeqTupleClose, C::PWS),
				Exact(C::SeqTupleSep, C::PWS),
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionTuple>(
						value->GetRange(),
						std::dynamic_pointer_cast<MultiValue>(mv[0])->Extract<std::shared_ptr<IExpression>>()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PUpdate = Map(
			All(
				C::Tokens{
					Exact(C::SeqRecordOpen, C::PWS),
					PExpression,
					Exact(C::SeqRecordUpdate, C::PWS),
					Some(
						All(C::Tokens{C::PIdentifier, Exact(C::SeqRecordBind, C::PWS), PExpression}, C::PWS),
						nullptr,  // prefix
						nullptr,  // suffix
						Exact(C::SeqRecordSep, C::PWS),
						C::PWS
					),
					Exact(C::SeqRecordClose, C::PWS)
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionUpdate>(
						mv[0]->GetRange(),
						std::dynamic_pointer_cast<ExpressionValue>(mv[1])->GetValue(),
						std::dynamic_pointer_cast<MultiValue>(mv[3])->Extract<ExpressionUpdate::Field>(
							[](std::shared_ptr<ITokenValue> const& fieldValue) -> ExpressionUpdate::Field {
								C::Values mv = std::dynamic_pointer_cast<MultiValue>(fieldValue)->GetValues();
								return ExpressionUpdate::Field{
									.range = fieldValue->GetRange(),
									.name = std::dynamic_pointer_cast<C::IdentifierValue>(mv[0])->GetValue(),
									.nameRange = mv[0]->GetRange(),
									.value = std::dynamic_pointer_cast<ExpressionValue>(mv[2])->GetValue()
								};
							}
						)
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PVar =
			Map(C::PQualifiedIdentifier, [](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::make_shared<ExpressionValue>(
					value->GetRange(),
					std::make_shared<ExpressionVar>(
						value->GetRange(),
						std::dynamic_pointer_cast<C::QualifiedIdentifierValue>(value)->GetValue()
					)
				);
			});

	private:
		inline static ForwardDeclarationToken::Replacement PExpressionReplacement{
			PExpression,
			C::Tokens{
				PAccessor,
				PAccess,
				PApply,
				PBinOp,
				PConst,
				PIf,
				PInfix,
				PLambda,
				PLet,
				PList,
				PNegate,
				PRecord,
				PSelect,
				PTuple,
				PUpdate,
				PVar
			}
		};

		inline static ForwardDeclarationToken::Replacement PLetReplacement{PLet, C::Tokens{PLetFunction, PLetValue}};
	};
}
