#pragma once

#include "../_external.hh"
#include "ast_types.hh"
#include "parser_common.hh"

namespace funcc::nar {
	using namespace funcc::parser;

	class TypeParser {
		using C = CommonParser;

	public:
		using TypeValue = Value<std::shared_ptr<nar::IType>>;

		static std::shared_ptr<IToken> PType;

		inline static std::shared_ptr<IToken> PTypeAnnotation = Map(
			All(C::Tokens{Exact(C::SeqTypeAnnotation, C::PWS), PType}, C::PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::dynamic_pointer_cast<MultiValue>(value)->GetValues()[1];
			}
		);

		// TODO: support names for funtion parameter types
		inline static std::shared_ptr<IToken> PFunctionType = Map(
			All(
				C::Tokens{
					Some(
						PType,
						Exact(C::SeqFuncOpen, C::PWS),
						Exact(C::SeqFuncClose, C::PWS),
						Exact(C::SeqFuncSep, C::PWS),
						C::PWS
					),
					PTypeAnnotation,
				},
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::make_shared<TypeValue>(
					value->GetRange(),
					std::make_shared<nar::FunctionType>(
						value->GetRange(),
						std::dynamic_pointer_cast<MultiValue>(value)->Extract<std::shared_ptr<nar::IType>>(),
						std::dynamic_pointer_cast<TypeValue>(
							std::dynamic_pointer_cast<MultiValue>(value)->GetValues()[1]
						)
							->GetValue()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PNamedType = Map(
			All(C::Tokens{C::PIdentifier, Optional(C::PTypeParameters)}, C::PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				std::vector<std::shared_ptr<ITokenValue>> mv = std::dynamic_pointer_cast<MultiValue>(value)->GetValues(
				);
				return std::make_shared<TypeValue>(
					value->GetRange(),
					std::make_shared<nar::NamedType>(
						value->GetRange(),
						std::dynamic_pointer_cast<C::IdentifierValue>(mv[0])->GetValue(),
						value->GetRange(),
						mv[1]->IsSkipped()
							? std::vector<std::shared_ptr<IType>>{}
							: std::dynamic_pointer_cast<MultiValue>(mv[1])->Extract<std::shared_ptr<IType>>()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PVariantType =
			Map(C::PIdentifier, [](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				nar::Identifier id = std::dynamic_pointer_cast<C::IdentifierValue>(value)->GetValue();
				if (std::islower(id[0])) {
					return std::make_shared<TypeValue>(
						value->GetRange(),
						std::make_shared<VarintType>(value->GetRange(), id)
					);
				}
				return std::make_shared<ErrorValue>(
					value->GetRange(),
					"Expected lowercase identifier for variable type"
				);
			});

		inline static std::shared_ptr<IToken> PRecordType = Map(
			Some(
				All(C::Tokens{C::PIdentifier, PTypeAnnotation}, C::PWS),
				Exact(C::SeqRecordOpen, C::PWS),
				Exact(C::SeqRecordClose, C::PWS),
				Exact(C::SeqRecordSep, C::PWS),
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::make_shared<TypeValue>(
					value->GetRange(),
					std::make_shared<nar::RecordType>(
						value->GetRange(),
						std::dynamic_pointer_cast<MultiValue>(value)->Extract<RecordField>(
							[](std::shared_ptr<ITokenValue> const& value) -> RecordField {
								std::vector<std::shared_ptr<ITokenValue>> mv =
									std::dynamic_pointer_cast<MultiValue>(value)->GetValues();
								return RecordField{
									.name = std::dynamic_pointer_cast<C::IdentifierValue>(mv[0])->GetValue(),
									.nameRange = mv[0]->GetRange(),
									.type = std::dynamic_pointer_cast<TypeValue>(mv[1])->GetValue(),
								};
							}
						)
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PTupleType = Map(
			Some(
				PType,
				Exact(C::SeqTupleOpen, C::PWS),
				Exact(C::SeqTupleClose, C::PWS),
				Exact(C::SeqTupleSep, C::PWS),
				C::PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::make_shared<TypeValue>(
					value->GetRange(),
					std::make_shared<nar::TupleType>(
						value->GetRange(),
						std::dynamic_pointer_cast<MultiValue>(value)->Extract<std::shared_ptr<nar::IType>>()
					)
				);
			}
		);

		inline static std::shared_ptr<IToken> PUnitType = Map(
			Exact(C::SeqUnitType, C::PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::make_shared<TypeValue>(
					value->GetRange(),
					std::make_shared<nar::UnitType>(value->GetRange())
				);
			}
		);
	};

	inline std::shared_ptr<IToken> TypeParser::PType = OneOf(
		CommonParser::Tokens{
			TypeParser::PFunctionType,
			TypeParser::PNamedType,
			TypeParser::PVariantType,
			TypeParser::PRecordType,
			TypeParser::PTupleType,
			TypeParser::PUnitType
		},
		CommonParser::PWS
	);
}
