#pragma once

#include "../_external.hh"
#include "ast_patterns.hh"
#include "parser_common.hh"
#include "parser_type.hh"

namespace funcc::nar {
	using namespace funcc::parser;

	class PatternParser {
		using C = CommonParser;
		using T = TypeParser;

	public:
		struct FunctionSignature {
			Range range;
			Identifier name;
			Range nameRange;
			std::vector<std::shared_ptr<IPattern>> params;
			std::shared_ptr<IType> returnType;
		};

		using PatternValue = Value<std::shared_ptr<nar::IPattern>>;
		using FunctionSignatureValue = funcc::parser::Value<FunctionSignature>;

		inline static std::shared_ptr<IToken> PPattern = ForwardDeclaration();

		inline static std::shared_ptr<IToken> PAlias = Map(
			All(C::Tokens{PPattern, Exact(C::KwAs, C::PWS), C::PIdentifier, Optional(T::PTypeAnnotation)}, C::PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<PatternValue>(
					value->GetRange(),
					std::make_shared<nar::PatternAlias>(
						value->GetRange(),
						mv[3]->IsSkipped() ? nullptr : std::dynamic_pointer_cast<T::TypeValue>(mv[3])->GetValue(),
						std::dynamic_pointer_cast<C::IdentifierValue>(mv[2])->GetValue(),
						std::dynamic_pointer_cast<PatternValue>(mv[0])->GetValue()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PAny = Map(
			Exact(C::SeqPatternAny, C::PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::make_shared<PatternValue>(
					value->GetRange(),
					std::make_shared<nar::PatternAny>(value->GetRange(), nullptr)
				);
			}
		);

		inline static std::shared_ptr<IToken> PCons = Map(
			All(C::Tokens{PPattern, Exact(C::SeqCons, C::PWS), PPattern, Optional(T::PTypeAnnotation)}, C::PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<PatternValue>(
					value->GetRange(),
					std::make_shared<nar::PatternCons>(
						value->GetRange(),
						mv[3]->IsSkipped() ? nullptr : std::dynamic_pointer_cast<T::TypeValue>(mv[3])->GetValue(),
						std::dynamic_pointer_cast<PatternValue>(mv[0])->GetValue(),
						std::dynamic_pointer_cast<PatternValue>(mv[2])->GetValue()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PConst = Map(
			All(C::Tokens{C::PConst, Optional(T::PTypeAnnotation)}, C::PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<PatternValue>(
					value->GetRange(),
					std::make_shared<nar::PatternConst>(
						value->GetRange(),
						mv[1]->IsSkipped() ? nullptr : std::dynamic_pointer_cast<T::TypeValue>(mv[1])->GetValue(),
						std::dynamic_pointer_cast<C::ConstValue>(mv[0])->GetValue()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PNamed = Map(
			All(C::Tokens{C::PIdentifier, Optional(T::PTypeAnnotation)}, C::PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<PatternValue>(
					value->GetRange(),
					std::make_shared<nar::PatternNamed>(
						value->GetRange(),
						mv[1]->IsSkipped() ? nullptr : std::dynamic_pointer_cast<T::TypeValue>(mv[1])->GetValue(),
						std::dynamic_pointer_cast<C::IdentifierValue>(mv[0])->GetValue()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PDataConstructor = Map(
			All(
				C::Tokens{
					C::PQualifiedIdentifier,
					Some(
						PPattern,
						Exact(C::SeqFuncOpen, C::PWS),
						Exact(C::SeqFuncClose, C::PWS),
						Exact(C::SeqFuncSep, C::PWS),
						C::PWS,
						nullptr,
						true
					),
					Optional(T::PTypeAnnotation)
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<PatternValue>(
					value->GetRange(),
					std::make_shared<nar::PatternDataConstructor>(
						value->GetRange(),
						mv[2]->IsSkipped() ? nullptr : std::dynamic_pointer_cast<T::TypeValue>(mv[2])->GetValue(),
						std::dynamic_pointer_cast<C::IdentifierValue>(mv[0])->GetValue(),
						mv[0]->GetRange(),
						std::dynamic_pointer_cast<MultiValue>(mv[1])->Extract<std::shared_ptr<IPattern>>()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PList = Map(
			All(
				C::Tokens{
					Some(
						PPattern,
						Exact(C::SeqListOpen, C::PWS),
						Exact(C::SeqListClose, C::PWS),
						Exact(C::SeqListSep, C::PWS),
						C::PWS,
						nullptr,
						true
					),
					Optional(T::PTypeAnnotation)
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<PatternValue>(
					value->GetRange(),
					std::make_shared<nar::PatternList>(
						value->GetRange(),
						mv[1]->IsSkipped() ? nullptr : std::dynamic_pointer_cast<T::TypeValue>(mv[1])->GetValue(),
						std::dynamic_pointer_cast<MultiValue>(mv[0])->Extract<std::shared_ptr<IPattern>>()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PRecord = Map(
			All(
				C::Tokens{
					Some(
						C::PIdentifier,
						Exact(C::SeqRecordOpen, C::PWS),
						Exact(C::SeqRecordClose, C::PWS),
						Exact(C::SeqRecordSep, C::PWS),
						C::PWS
					),
					Optional(T::PTypeAnnotation),
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<PatternValue>(
					value->GetRange(),
					std::make_shared<nar::PatternRecord>(
						value->GetRange(),
						mv[1]->IsSkipped() ? nullptr : std::dynamic_pointer_cast<T::TypeValue>(mv[1])->GetValue(),
						std::dynamic_pointer_cast<MultiValue>(mv[0])->Extract<std::pair<Range, Identifier>>(
							[](std::shared_ptr<ITokenValue> const& value) -> std::pair<Range, Identifier> {
								return std::make_pair(
									value->GetRange(),
									std::dynamic_pointer_cast<C::IdentifierValue>(value)->GetValue()
								);
							}
						)
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PTuple = Map(
			All(
				C::Tokens{
					Some(
						PPattern,
						Exact(C::SeqTupleOpen, C::PWS),
						Exact(C::SeqTupleClose, C::PWS),
						Exact(C::SeqTupleSep, C::PWS),
						C::PWS
					),
					Optional(T::PTypeAnnotation)
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<PatternValue>(
					value->GetRange(),
					std::make_shared<nar::PatternTuple>(
						value->GetRange(),
						mv[1]->IsSkipped() ? nullptr : std::dynamic_pointer_cast<T::TypeValue>(mv[1])->GetValue(),
						std::dynamic_pointer_cast<MultiValue>(mv[0])->Extract<std::shared_ptr<IPattern>>()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PFunctionSignature = Map(
			All(
				C::Tokens{
					C::PIdentifier,
					Optional(Some(
						PPattern,
						Exact(C::SeqFuncOpen, C::PWS),
						Exact(C::SeqFuncClose, C::PWS),
						Exact(C::SeqFuncSep, C::PWS),
						C::PWS
					)),
					Optional(T::PTypeAnnotation),
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				C::Values mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
				return std::make_shared<FunctionSignatureValue>(
					value->GetRange(),
					FunctionSignature{
						.name = std::dynamic_pointer_cast<C::IdentifierValue>(mv[0])->GetValue(),
						.nameRange = mv[0]->GetRange(),
						.params = mv[1]->IsSkipped()
							? std::vector<std::shared_ptr<IPattern>>{}
							: std::dynamic_pointer_cast<MultiValue>(mv[1])->Extract<std::shared_ptr<IPattern>>(),
						.returnType =
							mv[2]->IsSkipped() ? nullptr : std::dynamic_pointer_cast<T::TypeValue>(mv[2])->GetValue(),
					}
				);
			}
		);

	private:
		inline static ForwardDeclarationToken::Replacement PPatternReplacement{
			PPattern,
			C::Tokens{PAlias, PAny, PCons, PConst, PNamed, PDataConstructor, PList, PRecord, PTuple}
		};
	};
}
