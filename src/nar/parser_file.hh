#pragma once

#include "../_external.hh"
#include "../parser.hh"
#include "ast_common.hh"
#include "ast_declarations.hh"
#include "ast_expressions.hh"
#include "ast_patterns.hh"
#include "ast_types.hh"
#include "parser_common.hh"
#include "parser_expression.hh"
#include "parser_pattern.hh"
#include "parser_type.hh"

namespace funcc::nar {
	using namespace funcc::parser;

	class FileParser {
		using C = CommonParser;
		using T = TypeParser;
		using E = ExpressionParser;
		using P = PatternParser;

	public:
		using ImportValue = Value<nar::Import>;
		using InfixValue = Value<nar::Infix>;
		using AliasValue = Value<nar::Alias>;
		using FunctionValue = Value<nar::Function>;
		using DataValue = Value<nar::Data>;
		using DataConstructorParameterValue = Value<nar::DataConstructorParameter>;
		using DataConstructorParametersValue = Value<std::vector<nar::DataConstructorParameter>>;
		using DataConstructorValue = Value<DataConstructor>;
		using ExpressionValue = Value<std::shared_ptr<nar::IExpression>>;
		using FileValue = Value<nar::File>;

		/*
		constexpr static std::string_view SeqBracketsOpen = "[";
		constexpr static std::string_view SeqBracketsClose = "]";
		constexpr static std::string_view SeqBracesOpen = "{";
		constexpr static std::string_view SeqBracesClose = "}";
		constexpr static std::string_view SeqComma = ",";
		constexpr static std::string_view SeqColon = ":";
		constexpr static std::string_view SeqEqual = "=";
		constexpr static std::string_view SeqBar = "|";
		constexpr static std::string_view SeqLambda = "\\(";
		constexpr static std::string_view SeqLambdaBind = "->";
		constexpr static std::string_view SeqCaseBind = "->";
		constexpr static std::string_view SeqInfixChars = "!#$%&*+-/:;<=>?^|~`";*/

		inline static std::shared_ptr<IToken> PModule = Map(
			All(C::Tokens{Exact(C::KwModule, C::PWS), C::PQualifiedIdentifier}, C::PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::dynamic_pointer_cast<MultiValue>(value)->GetValues()[1];
			}
		);

		inline static std::shared_ptr<IToken> PImportExposing = OneOf(
			C::Tokens{
				Exact(C::SeqExposingAll, C::PWS),
				Some(
					C::PIdentifier,
					Exact(C::SeqImportListOpen, C::PWS),
					Exact(C::SeqImportListClose, C::PWS),
					Exact(C::SeqImportListSep, C::PWS),
					C::PWS
				)
			},
			C::PWS
		);

		inline static std::shared_ptr<IToken> PImport = Map(
			All(
				C::Tokens{
					Exact(C::KwImport, C::PWS),
					C::PQualifiedIdentifier,
					Optional(All(C::Tokens{Exact(C::KwAs, C::PWS), C::PIdentifier}, C::PWS)),
					Optional(All(C::Tokens{Exact(C::KwExposing, C::PWS), PImportExposing}, C::PWS))
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values const& mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				Import import{
					.range = value->GetRange(),
					.module = std::dynamic_pointer_cast<C::QualifiedIdentifierValue>(mv[1])->GetValue(),
				};
				if (mv[2]->GetKind() != ValueKind::SkippedOptional) {
					import.alias = std::dynamic_pointer_cast<C::IdentifierValue>(mv[2])->GetValue();
				}

				if (mv[3]->GetKind() != ValueKind::SkippedOptional) {
					std::shared_ptr<ITokenValue> expose = std::dynamic_pointer_cast<MultiValue>(mv[3])->GetValues()[1];
					if (expose->GetKind() == ValueKind::Exact) {
						import.exposeAll = true;
					} else {
						import.expose = std::dynamic_pointer_cast<MultiValue>(expose)->Extract<nar::Identifier>();
					}
				}
				return std::make_shared<ImportValue>(value->GetRange(), std::move(import));
			}
		);

		inline static std::shared_ptr<IToken> PImports = Repeat(
			Exact(C::KwImport, C::PWS),
			// TODO: extract to import member
			PImport,
			C::PWS,
			true
		);

		inline static std::shared_ptr<IToken> PAlias = Map(
			All(
				C::Tokens{
					Exact(C::KwAlias, C::PWS),
					Optional(Exact(C::KwHidden, C::PWS)),
					Optional(
						Exact(C::KwNative, C::PWS),
						All(C::Tokens{C::PIdentifier, Optional(T::PTypeParameters)}, C::PWS),
						All(
							C::Tokens{
								C::PIdentifier,
								Optional(T::PTypeParameters),
								Exact(C::SeqAliasBind, C::PWS),
								T::PType
							},
							C::PWS
						)
					)
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				bool hidden = !mv[1]->IsSkipped();
				mv = std::dynamic_pointer_cast<MultiValue>(mv[2])->GetValues();

				return std::make_shared<AliasValue>(
					value->GetRange(),
					nar::Alias{
						value->GetRange(),
						std::dynamic_pointer_cast<C::IdentifierValue>(mv[0])->GetValue(),
						mv[0]->GetRange(),
						hidden,
						mv.size() < 4 ? nullptr : std::dynamic_pointer_cast<T::TypeValue>(mv[3])->GetValue(),
						mv[1]->IsSkipped()
							? std::vector<std::shared_ptr<nar::IType>>{}
							: std::dynamic_pointer_cast<MultiValue>(mv[1])->Extract<std::shared_ptr<nar::IType>>(),
					}
				);
			}
		);

		// TODO: proposal to think about changing the syntax of infix operator definition, eg `infix (++): left<5> = fn`
		inline static std::shared_ptr<IToken> PInfix = Map(
			All(
				C::Tokens{
					Exact(C::KwInfix, C::PWS),
					Optional(Exact(C::KwHidden, C::PWS)),
					C::PWrappedInfixIdentifier,
					Exact(C::SeqInfixTypeDecl, C::PWS),
					Exact(C::SeqInfixTypeOpen, C::PWS),
					OneOf(
						C::Tokens{Exact(C::KwLeft, C::PWS), Exact(C::KwRight, C::PWS), Exact(C::KwNon, C::PWS)},
						C::PWS
					),
					NumberLiteral(C::PWS),
					Exact(C::SeqInfixTypeClose, C::PWS),
					Exact(C::SeqInfixBind, C::PWS),
					C::PIdentifier,
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				std::shared_ptr<NumberLiteralValue> precedence = std::dynamic_pointer_cast<NumberLiteralValue>(mv[6]);
				if (!precedence->IsInteger()) {
					return std::make_shared<ErrorValue>(
						precedence->GetRange(),
						"Expected integer for infix operator precedence"
					);
				}
				std::shared_ptr<SimpleValue> associativity = std::dynamic_pointer_cast<SimpleValue>(mv[5]);
				Associativity assoc = Associativity::None;
				if (associativity->GetValue() == C::KwLeft) {
					assoc = Associativity::Left;
				} else if (associativity->GetValue() == C::KwRight) {
					assoc = Associativity::Right;
				} else if (associativity->GetValue() == C::KwNon) {
					assoc = Associativity::None;
				}

				return std::make_shared<InfixValue>(
					value->GetRange(),
					nar::Infix{
						value->GetRange(),
						std::dynamic_pointer_cast<C::InfixIdentifierValue>(mv[2])->GetValue(),
						mv[2]->GetRange(),
						!mv[1]->IsSkipped(),
						assoc,
						precedence->GetInteger(),
						std::dynamic_pointer_cast<C::IdentifierValue>(mv[9])->GetValue(),
					}
				);
			}
		);

		inline static std::shared_ptr<IToken> PDataConstructorParameter = Map(
			All(C::Tokens{Optional(C::PIdentifier, Exact(C::SeqTypeAnnotation, C::PWS)), T::PType}, C::PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<DataConstructorParameterValue>(
					value->GetRange(),
					nar::DataConstructorParameter{
						value->GetRange(),
						mv[0]->IsSkipped() ? nullptr : std::dynamic_pointer_cast<C::IdentifierValue>(mv[0])->GetValue(),
						mv[0]->GetRange(),
						std::dynamic_pointer_cast<T::TypeValue>(mv[1])->GetValue(),
					}
				);
			}
		);

		inline static std::shared_ptr<IToken> PDataConstructorParameters = Some(
			PDataConstructorParameter,
			Exact(C::SeqFuncOpen, C::PWS),
			Exact(C::SeqFuncClose, C::PWS),
			Exact(C::SeqFuncSep, C::PWS),
			C::PWS
		);

		inline static std::shared_ptr<IToken> PDataConstructor(bool first) {
			return Map(
				All(
					C::Tokens{
						first ? Optional(Exact(C::SeqDataConstructor, C::PWS)) : Exact(C::SeqDataConstructor, C::PWS),
						Optional(Exact(C::KwHidden, C::PWS)),
						C::PIdentifier,
						Optional(PDataConstructorParameters)
					},
					C::PWS
				),
				[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
					C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
					return std::make_shared<DataConstructorValue>(
						value->GetRange(),
						nar::DataConstructor{
							value->GetRange(),
							!mv[1]->IsSkipped(),
							std::dynamic_pointer_cast<C::IdentifierValue>(mv[2])->GetValue(),
							mv[2]->GetRange(),
							mv[3]->IsSkipped()
								? std::vector<nar::DataConstructorParameter>{}
								: std::dynamic_pointer_cast<MultiValue>(mv[3])->Extract<nar::DataConstructorParameter>(
								  ),
						}
					);
				}
			);
		}

		inline static std::shared_ptr<IToken> PData = Map(
			All(
				C::Tokens{
					Exact(C::KwData, C::PWS),
					Optional(Exact(C::KwHidden, C::PWS)),
					C::PIdentifier,
					Optional(T::PTypeParameters),
					Exact(C::SeqDataBind, C::PWS),
					PDataConstructor(true),
					Repeat(Exact(C::SeqDataConstructor, C::PWS), PDataConstructor(false), C::PWS, true),
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				std::vector<nar::DataConstructor> ctors =
					std::dynamic_pointer_cast<MultiValue>(mv[6])->Extract<nar::DataConstructor>();
				ctors.insert(ctors.begin(), std::dynamic_pointer_cast<DataConstructorValue>(mv[5])->GetValue());
				return std::make_shared<DataValue>(
					value->GetRange(),
					nar::Data{
						value->GetRange(),
						std::dynamic_pointer_cast<C::IdentifierValue>(mv[2])->GetValue(),
						mv[2]->GetRange(),
						!mv[1]->IsSkipped(),
						mv[3]->IsSkipped() ? std::vector<nar::Identifier>{}
										   : std::dynamic_pointer_cast<MultiValue>(mv[3])->Extract<nar::Identifier>(),
						std::move(ctors),
					}
				);
			}
		);

		inline static std::shared_ptr<IToken> PFunction = Map(
			All(
				C::Tokens{
					Exact(C::KwDef, C::PWS),
					Optional(Exact(C::KwHidden, C::PWS)),
					Optional(
						Exact(C::KwNative, C::PWS),
						OneOf(
							C::Tokens{
								All(C::Tokens{C::PIdentifier, T::PTypeAnnotation}, C::PWS),
								All(C::Tokens{P::PFunctionSignature}, C::PWS)
							},
							C::PWS
						),
						OneOf(
							C::Tokens{
								All(
									C::Tokens{
										C::PIdentifier,
										Optional(T::PTypeAnnotation),
										Exact(C::SeqFunctionBind, C::PWS),
										E::PExpression
									},
									C::PWS
								),
								All(C::Tokens{P::PFunctionSignature, Exact(C::SeqFunctionBind, C::PWS), E::PExpression},
									C::PWS)
							},
							C::PWS
						)
					),
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				bool isHidden = !mv[1]->IsSkipped();
				mv = std::dynamic_pointer_cast<MultiValue>(mv[2])->GetValues();

				Identifier name{};
				Range nameRange{};
				P::FunctionSignature signature{};
				std::shared_ptr<nar::IExpression> expr{};
				std::shared_ptr<IType> type{};

				switch (mv.size()) {
					case 1: {  // native function
						signature = std::dynamic_pointer_cast<P::FunctionSignatureValue>(mv[0])->GetValue();
						bool typed = signature.returnType != nullptr;
						for (auto const& param: signature.params) {
							if (!param->GetType()) {
								typed = false;
								break;
							}
						}

						if (!typed) {
							return std::make_shared<ErrorValue>(value->GetRange(), "Expected type annotation");
						}

						break;
					}
					case 2: {  // native constant
						name = std::dynamic_pointer_cast<C::IdentifierValue>(mv[0])->GetValue();
						nameRange = mv[0]->GetRange();
						if (mv[1]->IsSkipped()) {
							return std::make_shared<ErrorValue>(value->GetRange(), "Expected type annotation");
						}

						type = std::dynamic_pointer_cast<T::TypeValue>(mv[1])->GetValue();
						break;
					}
					case 3: {  // function
						signature = std::dynamic_pointer_cast<P::FunctionSignatureValue>(mv[0])->GetValue();
						expr = std::dynamic_pointer_cast<ExpressionValue>(mv[2])->GetValue();
						break;
					}
					case 4: {  // constant
						name = std::dynamic_pointer_cast<C::IdentifierValue>(mv[0])->GetValue();
						if (!mv[1]->IsSkipped()) {
							type = std::dynamic_pointer_cast<T::TypeValue>(mv[1])->GetValue();
						}
						nameRange = mv[0]->GetRange();
						expr = std::dynamic_pointer_cast<ExpressionValue>(mv[3])->GetValue();
					}
				}

				if (name.empty()) {
					name = signature.name;
					nameRange = signature.nameRange;

					std::vector<std::shared_ptr<IType>> paramsTypes;
					paramsTypes.reserve(signature.params.size());
					for (auto const& param: signature.params) {
						paramsTypes.push_back(param->GetType());
					}
					type = std::make_shared<nar::FunctionType>(
						signature.range,
						std::move(paramsTypes),
						signature.returnType
					);
				}

				return std::make_shared<FunctionValue>(
					value->GetRange(),
					nar::Function{
						value->GetRange(),
						name,
						nameRange,
						isHidden,
						signature.params,
						type,
						expr,
					}
				);
			}
		);

		inline static std::shared_ptr<IToken> PDeclarations = Repeat(
			OneOf(
				C::Tokens{
					Exact(C::KwAlias, C::PWS),
					Exact(C::KwInfix, C::PWS),
					Exact(C::KwData, C::PWS),
					Exact(C::KwDef, C::PWS)
				},
				C::PWS
			),
			OneOf(C::Tokens{PAlias, PInfix, PData, PFunction}, C::PWS),
			C::PWS,
			true
		);

		inline static std::shared_ptr<IToken> PFile = Map(
			All(C::Tokens{PModule, PImports, PDeclarations, Eof(C::PWS)}, C::PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();

				return std::make_shared<FileValue>(
					value->GetRange(),
					File{
						.module = std::dynamic_pointer_cast<C::QualifiedIdentifierValue>(mv[0])->GetValue(),
						.moduleRange = mv[0]->GetRange(),
						.imports = std::dynamic_pointer_cast<MultiValue>(mv[1])->Extract<nar::Import>(),
						.declarations =
							std::dynamic_pointer_cast<MultiValue>(mv[2])->Extract<std::shared_ptr<IDeclaration>>(),
					}
				);
			}
		);
	};
}
