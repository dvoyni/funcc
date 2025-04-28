#pragma once

#include "../api.hh"
#include "../parser.hh"
#include "ast.hh"
#include "expressions.hh"
#include "patterns.hh"

namespace funcc::nar {

	class ParsedFile final : public IParsedFile {
	public:
		~ParsedFile() override = default;

		Result<normalized::File> Normalize(Context const& ctx) const override {
			// TODO: impelement
		}
	};

	class NarParser {
		constexpr static std::string const KwModule{"module"};
		constexpr static std::string const KwImport{"import"};
		constexpr static std::string const KwAs{"as"};
		constexpr static std::string const KwExposing{"exposing"};
		constexpr static std::string const KwInfix{"infix"};
		constexpr static std::string const KwAlias{"alias"};
		constexpr static std::string const KwType{"type"};
		constexpr static std::string const KwDef{"def"};
		constexpr static std::string const KwHidden{"hidden"};
		constexpr static std::string const KwNative{"native"};
		constexpr static std::string const KwLeft{"left"};
		constexpr static std::string const KwRight{"right"};
		constexpr static std::string const KwNon{"non"};
		constexpr static std::string const KwIf{"if"};
		constexpr static std::string const KwThen{"then"};
		constexpr static std::string const KwElse{"else"};
		constexpr static std::string const KwLet{"let"};
		constexpr static std::string const KwIn{"in"};
		constexpr static std::string const KwSelect{"select"};
		constexpr static std::string const KwCase{"case"};
		constexpr static std::string const KwEnd{"end"};

		constexpr static std::string const SeqComment{"//"};
		constexpr static std::string const SeqCommentStart{"/*"};
		constexpr static std::string const SeqCommentEnd{"*/"};
		constexpr static std::string const SeqExposingAll{"*"};
		constexpr static std::string const SeqParenthesisOpen{"("};
		constexpr static std::string const SeqParenthesisClose{")"};
		constexpr static std::string const SeqBracketsOpen{"["};
		constexpr static std::string const SeqBracketsClose{"]"};
		constexpr static std::string const SeqBracesOpen{"{"};
		constexpr static std::string const SeqBracesClose{"}"};
		constexpr static std::string const SeqComma{","};
		constexpr static std::string const SeqColon{":"};
		constexpr static std::string const SeqEqual{"="};
		constexpr static std::string const SeqBar{"|"};
		constexpr static std::string const SeqUnderscore = {"_"};
		constexpr static std::string const SeqDot{"."};
		constexpr static std::string const SeqMinus{"-"};
		constexpr static std::string const SeqLambda{"\\("};
		constexpr static std::string const SeqLambdaBind{"->"};
		constexpr static std::string const SeqCaseBind{"->"};
		constexpr static std::string const SeqInfixChars{"!#$%&*+-/:;<=>?^|~`"};

		constexpr static std::string const SmbNewLine{"\n"};
		constexpr static std::string const SmbQuoteString{"\""};
		constexpr static std::string const SmbQuoteChar{"'"};
		constexpr static std::string const SmbEscape{"\\"};

		std::string m_input;

	public:
		NarParser(std::string input) :
			m_input{input} {}

		Result<IParsedFile> Parse() const {
			Parser parser{m_input};
		}

		[[nodiscard]] std::string const& GetInput() const {
			return m_input;
		}

	private:
		inline static void SkipComment(Parser parser) {
			while (parser.OneOf<void>([](Parser& parser) -> Result<void> {
				return parser.All<void, std::string_view, void>(
					[](Result<std::string_view>, Result<void>) -> Result<void> { return Result<void>{}; },
					[](Parser& parser) -> Result<std::string_view> { return parser.Exact(SeqComment); },
					[](Parser& parser) -> Result<void> {
						return parser.Read<void>(
							[](std::string_view, char32_t c) { return c != '\n'; },
							[](std::string_view) { return Result<void>(); }
						);
					}
				);
			}))
			{};
		}
	};
}
