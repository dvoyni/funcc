#pragma once

#include "../_external.hh"
#include "../parser.hh"
#include "ast_common.hh"

namespace funcc::nar {
	using namespace funcc::parser;

	class CommonParser {
	public:
		using Tokens = std::vector<std::shared_ptr<IToken>>;
		using Values = std::vector<std::shared_ptr<ITokenValue>>;
		using IdentifierValue = funcc::parser::Value<nar::Identifier>;
		using QualifiedIdentifierValue = funcc::parser::Value<nar::QualifiedIdentifier>;
		using InfixIdentifierValue = funcc::parser::Value<nar::InfixIdentifier>;
		using ConstValue = funcc::parser::Value<std::shared_ptr<IConst>>;

		constexpr static std::string_view KwModule = "module";
		constexpr static std::string_view KwImport = "import";
		constexpr static std::string_view KwAs = "as";
		constexpr static std::string_view KwExposing = "exposing";
		constexpr static std::string_view KwInfix = "infix";
		constexpr static std::string_view KwAlias = "alias";
		constexpr static std::string_view KwData = "type";	// TODO: change to "data"
		constexpr static std::string_view KwDef = "def";
		constexpr static std::string_view KwHidden = "hidden";
		constexpr static std::string_view KwNative = "native";
		constexpr static std::string_view KwLeft = "left";
		constexpr static std::string_view KwRight = "right";
		constexpr static std::string_view KwNon = "non";
		constexpr static std::string_view KwIf = "if";
		constexpr static std::string_view KwThen = "then";
		constexpr static std::string_view KwElse = "else";
		constexpr static std::string_view KwLet = "let";
		constexpr static std::string_view KwIn = "in";
		constexpr static std::string_view KwSelect = "select";
		constexpr static std::string_view KwCase = "case";
		constexpr static std::string_view KwEnd = "end";

		constexpr static std::string_view SeqComment = "//";
		constexpr static std::string_view SeqCommentStart = "/*";
		constexpr static std::string_view SeqCommentEnd = "*/";
		constexpr static std::string_view SeqExposingAll = "*";
		constexpr static std::string_view SeqImportListOpen = "(";
		constexpr static std::string_view SeqImportListClose = ")";
		constexpr static std::string_view SeqImportListSep = ",";
		constexpr static std::string_view SeqAliasBind = "=";
		// TODO: switch to "<" and ">" for type parameters
		constexpr static std::string_view SeqTypeParametersOpen = "[";
		constexpr static std::string_view SeqTypeParametersClose = "]";
		constexpr static std::string_view SeqTypeParametersSep = ",";
		constexpr static std::string_view SeqUnitType = "()";
		constexpr static std::string_view SeqTupleOpen = "(";
		constexpr static std::string_view SeqTupleClose = ")";
		constexpr static std::string_view SeqTupleSep = ",";
		constexpr static std::string_view SeqListOpen = "[";
		constexpr static std::string_view SeqListClose = "]";
		constexpr static std::string_view SeqListSep = ",";
		constexpr static std::string_view SeqTypeAnnotation = ":";
		constexpr static std::string_view SeqRecordOpen = "{";
		constexpr static std::string_view SeqRecordClose = "}";
		constexpr static std::string_view SeqRecordSep = ",";
		constexpr static std::string_view SeqRecordBind = "=";
		constexpr static std::string_view SeqRecordUpdate = "|";
		constexpr static std::string_view SeqFuncOpen = "(";
		constexpr static std::string_view SeqFuncClose = ")";
		constexpr static std::string_view SeqFuncSep = ",";
		constexpr static std::string_view SeqInfixOpen = "(";
		constexpr static std::string_view SeqInfixClose = ")";
		constexpr static std::string_view SeqInfixTypeDecl = ":";
		constexpr static std::string_view SeqInfixTypeOpen = "(";
		constexpr static std::string_view SeqInfixTypeClose = ")";
		constexpr static std::string_view SeqInfixBind = "=";
		constexpr static std::string_view SeqDataBind = "=";
		constexpr static std::string_view SeqDataConstructor = "|";
		constexpr static std::string_view SeqFunctionBind = "=";
		constexpr static std::string_view SeqPatternAny = "_";
		constexpr static std::string_view SeqCons = "|";
		constexpr static std::string_view SeqStringPrefix = "\"";
		constexpr static std::string_view SeqStringSuffix = "\"";
		constexpr static std::string_view SeqStringEscape = "\\";
		constexpr static std::string_view SeqCharPrefix = "'";
		constexpr static std::string_view SeqCharSuffix = "'";
		constexpr static std::string_view SeqCharEscape = "\\";
		constexpr static std::string_view SeqAccessor = ".";
		constexpr static std::string_view SeqLambdaSignature = "\\(";
		constexpr static std::string_view SeqLambdaBind = "->";
		constexpr static std::string_view SeqNegate = "-";
		constexpr static std::string_view SeqCaseBind = "->";

		constexpr static std::string::value_type SmbIdentifierSeparator = '.';
		constexpr static std::string_view SmbIdentifier =
			"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_`";
		constexpr static std::string_view SmbIdentifierNotFirst = "0123456789_`";
		constexpr static std::string_view SmbInfixIdentifier = "!#$%&*+-/:;<=>?^|~`";

		inline static std::shared_ptr<IToken> PWS = IgnoreAny(
			Tokens{
				WhiteSpace(),
				SingleLineComment(SeqComment, nullptr),
				MultiLineComment(SeqCommentStart, SeqCommentEnd, nullptr)
			},
			nullptr
		);

		inline static std::shared_ptr<IToken> PQualifiedIdentifier = Map(
			Entity(
				[](std::string_view const& acc, uint32_t next, bool& outIsValid, bool& outIsComplete) {
					outIsComplete = next != SmbIdentifierSeparator &&
						SmbIdentifier.find(next) == std::string_view::npos;

					if (outIsComplete) {
						outIsValid = !acc.empty();

						for (size_t i = 0; i < acc.length() && outIsValid; ++i) {
							if (i == 0 || acc[i] == SmbIdentifierSeparator) {
								outIsValid |= (i < acc.length() - 1) || (acc[i + 1] != SmbIdentifierSeparator) ||
									(SmbIdentifierNotFirst.find(acc[i + 1]) == std::string_view::npos);
							}
						}
					}
				},
				PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				std::string_view acc = std::dynamic_pointer_cast<SimpleValue>(value)->GetValue();
				return std::make_shared<QualifiedIdentifierValue>(value->GetRange(), acc);
			}
		);

		inline static std::shared_ptr<IToken> PIdentifier = Map(
			Entity(
				[](std::string_view const& acc, uint32_t next, bool& outIsValid, bool& outIsComplete) {
					outIsComplete = SmbIdentifier.find(next) == std::string_view::npos;

					if (outIsComplete) {
						outIsValid = !acc.empty() && (SmbIdentifierNotFirst.find(acc[0]) == std::string_view::npos);
					}
				},
				PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				std::string_view acc = std::dynamic_pointer_cast<SimpleValue>(value)->GetValue();
				return std::make_shared<IdentifierValue>(value->GetRange(), acc);
			}
		);

		inline static std::shared_ptr<IToken> PInfixIdentifier = Map(
			Entity(
				[](std::string_view const& acc, uint32_t next, bool& outIsValid, bool& outIsComplete) {
					outIsComplete = SmbIdentifier.find(next) == std::string_view::npos;

					if (outIsComplete) {
						outIsValid = !acc.empty() && (SmbIdentifierNotFirst.find(acc[0]) == std::string_view::npos);
					}
				},
				PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				std::string_view acc = std::dynamic_pointer_cast<SimpleValue>(value)->GetValue();
				return std::make_shared<InfixIdentifierValue>(value->GetRange(), acc);
			}
		);

		inline static std::shared_ptr<IToken> PWrappedInfixIdentifier = Map(
			Entity(
				[](std::string_view const& acc, uint32_t next, bool& outIsValid, bool& outIsComplete) {
					outIsComplete = SmbIdentifier.find(next) == std::string_view::npos &&
						SeqInfixOpen.find(next) == std::string_view::npos &&
						SeqInfixClose.find(next) == std::string_view::npos;

					if (outIsComplete) {
						outIsValid = !acc.empty() && acc.find(SeqInfixOpen) == 0 &&
							acc.find(SeqInfixClose) == acc.length() - SeqInfixClose.length() &&
							acc.find(SeqInfixOpen, 1) == std::string_view::npos;
					}
				},
				PWS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				std::string_view acc = std::dynamic_pointer_cast<SimpleValue>(value)->GetValue();
				return std::make_shared<InfixIdentifierValue>(value->GetRange(), acc.substr(1, acc.length() - 2));
			}
		);

		inline static std::shared_ptr<IToken> PTypeParameters = Some(
			PIdentifier,
			Exact(SeqTypeParametersOpen, PWS),
			Exact(SeqTypeParametersClose, PWS),
			Exact(SeqTypeParametersSep, PWS),
			PWS
		);

		inline static std::shared_ptr<IToken> PConstChar = Map(
			StringLiteral(SeqCharPrefix, SeqCharSuffix, SeqCharEscape, PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				std::string_view acc = std::dynamic_pointer_cast<SimpleValue>(value)->GetValue();
				acc =
					acc.substr(SeqCharPrefix.length(), acc.length() - SeqCharPrefix.length() - SeqCharSuffix.length());
				if (acc.length() == 1) {
					return std::make_shared<ErrorValue>(value->GetRange(), "Expected single character");
				}
				return std::make_shared<ConstValue>(value->GetRange(), std::make_shared<ConstChar>(acc[0]));
			}
		);

		inline static std::shared_ptr<IToken> PConstInt =
			Map(NumberLiteral(PWS), [](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				std::shared_ptr<NumberLiteralValue> number = std::dynamic_pointer_cast<NumberLiteralValue>(value);
				if (!number->IsInteger()) {
					return std::make_shared<ErrorValue>(value->GetRange(), "Expected integer");
				}
				return std::make_shared<ConstValue>(
					value->GetRange(),
					std::make_shared<ConstInt>(number->GetInteger())
				);
			});

		inline static std::shared_ptr<IToken> PConstFloat =
			Map(NumberLiteral(PWS), [](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				std::shared_ptr<NumberLiteralValue> number = std::dynamic_pointer_cast<NumberLiteralValue>(value);
				if (!number->IsFloat()) {
					return std::make_shared<ErrorValue>(value->GetRange(), "Expected float");
				}
				return std::make_shared<ConstValue>(
					value->GetRange(),
					std::make_shared<ConstFloat>(number->GetFloat())
				);
			});

		inline static std::shared_ptr<IToken> PConstString = Map(
			StringLiteral(SeqStringPrefix, SeqStringSuffix, SeqStringEscape, PWS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				std::string_view acc = std::dynamic_pointer_cast<SimpleValue>(value)->GetValue();
				acc = acc.substr(
					SeqStringPrefix.length(),
					acc.length() - SeqStringPrefix.length() - SeqStringSuffix.length()
				);
				return std::make_shared<ConstValue>(value->GetRange(), std::make_shared<ConstString>(TString{acc}));
			}
		);

		inline static std::shared_ptr<IToken> PConstUnit =
			Map(Exact(SeqUnitType, PWS), [](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				return std::make_shared<ConstValue>(value->GetRange(), std::make_shared<ConstUnit>());
			});

		inline static std::shared_ptr<IToken> PConst =
			OneOf(Tokens{PConstChar, PConstFloat, PConstInt, PConstString, PConstUnit}, PWS);
	};
}
