#pragma once

#include "../parser.hh"
#include "ast_common.hh"
#include "ast_expressions.hh"
#include "ast_patterns.hh"

using namespace funcc::parser;

namespace funcc::nar {
	class NarParser {
		using Tokens = std::vector<std::shared_ptr<IToken>>;
		using IdentifierValue = Value<nar::Identifier>;
		using QualifiedIdentifierValue = Value<nar::QualifiedIdentifier>;
		using ImportValue = Value<nar::Import>;
		using FileValue = Value<nar::File>;

	public:
		constexpr static std::string_view KwModule = "module";
		constexpr static std::string_view KwImport = "import";
		constexpr static std::string_view KwAs = "as";
		constexpr static std::string_view KwExposing = "exposing";
		constexpr static std::string_view KwInfix = "infix";
		constexpr static std::string_view KwAlias = "alias";
		constexpr static std::string_view KwType = "type";
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

		constexpr static std::string_view SeqComment = "//";
		constexpr static std::string_view SeqCommentStart = "/*";
		constexpr static std::string_view SeqCommentEnd = "*/";
		constexpr static std::string_view SeqExposingAll = "*";
		constexpr static std::string_view SeqParenthesisOpen = "(";
		constexpr static std::string_view SeqParenthesisClose = ")";
		constexpr static std::string_view SeqItemSeparator = ",";

		constexpr static std::string::value_type SmbIdentifierSeparator = '.';
		constexpr static std::string_view SmbIdentifier =
			"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_`";
		constexpr static std::string_view SmbIdentifierNotFirst = "0123456789_`";

		inline static std::shared_ptr<IToken> WS = IgnoreAny(
			Tokens{WhiteSpace(), SingleLineComment(SeqComment), MultiLineComment(SeqCommentStart, SeqCommentEnd)}
		);

		inline static std::shared_ptr<IToken> QualifiedIdentifier = Entity(
			[](std::string_view const& acc, uint32_t next, bool& outIsValid, bool& outIsComplete) {
				outIsComplete = next != SmbIdentifierSeparator && SmbIdentifier.find(next) == std::string_view::npos;

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
			WS
		);

		inline static std::shared_ptr<IToken> Identifier = Entity(
			[](std::string_view const& acc, uint32_t next, bool& outIsValid, bool& outIsComplete) {
				outIsComplete = SmbIdentifier.find(next) == std::string_view::npos;

				if (outIsComplete) {
					outIsValid = !acc.empty() && (SmbIdentifierNotFirst.find(acc[0]) == std::string_view::npos);
				}
			},
			WS
		);

		inline static std::shared_ptr<IToken> Module = Map(
			All(Tokens{Exact(KwModule), QualifiedIdentifier}, WS),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				std::shared_ptr<MultiValue> mv = std::static_pointer_cast<MultiValue>(value);
				std::shared_ptr<SimpleValue> name = std::static_pointer_cast<SimpleValue>(mv->GetValues()[1]);
				return std::make_shared<QualifiedIdentifierValue>(name->GetRange(), name->GetValue());
			}
		);

		inline static std::shared_ptr<IToken> Import = Map(
			All(
				Tokens{
					Exact(KwImport),
					QualifiedIdentifier,
					Optional(All(Tokens{Exact(KwAs), Identifier}, WS)),
					Optional(All(
						Tokens{
							Exact(KwExposing),
							OneOf(
								Tokens{
									Exact(SeqExposingAll),
									Some(
										Exact(SeqParenthesisOpen),
										Identifier,
										Exact(SeqItemSeparator),
										Exact(SeqParenthesisClose),
										WS
									)
								}
							)
						},
						WS
					))
				},
				WS
			),
			[](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				std::vector<std::shared_ptr<ITokenValue>> const& mv =
					std::static_pointer_cast<MultiValue>(value)->GetValues();
				nar::Import import{
					.range = value->GetRange(),
					.module = std::static_pointer_cast<QualifiedIdentifierValue>(mv[1])->GetValue(),
				};
				if (mv[2]->GetKind() != ValueKind::SkippedOptional) {
					import.alias = std::static_pointer_cast<IdentifierValue>(mv[2])->GetValue();
				}

				if (mv[3]->GetKind() != ValueKind::SkippedOptional) {
					std::shared_ptr<ITokenValue> expose = std::static_pointer_cast<MultiValue>(mv[3])->GetValues()[1];
					if (expose->GetKind() == ValueKind::Exact) {
						import.exposeAll = true;
					} else {
						import.expose = std::vector<nar::Identifier>();
						for (auto const& item: std::static_pointer_cast<MultiValue>(expose)->GetValues()) {
							import.expose.push_back(std::static_pointer_cast<IdentifierValue>(item)->GetValue());
						}
					}
				}
				return std::make_shared<ImportValue>(value->GetRange(), std::move(import));
			}
		);

		inline static std::shared_ptr<IToken> File =
			Map(All(Tokens{Module}), [](std::shared_ptr<ITokenValue> const& value) -> std::shared_ptr<ITokenValue> {
				std::shared_ptr<MultiValue> mv = std::static_pointer_cast<MultiValue>(value);
				std::shared_ptr<QualifiedIdentifierValue> module =
					std::static_pointer_cast<QualifiedIdentifierValue>(mv->GetValues()[0]);

				return std::make_shared<FileValue>(
					module->GetRange(),
					nar::File{
						.module = module->GetValue(),
						.moduleRange = module->GetRange(),
						.imports = {},
						.aliases = {},
						.infixDefinitions = {},
						.functionDefinitions = {},
						.dataTypes = {},
					}
				);
			});
	};
}
