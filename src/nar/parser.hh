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

		constexpr static std::string_view SeqComment = "//";
		constexpr static std::string_view SeqCommentStart = "/*";
		constexpr static std::string_view SeqCommentEnd = "*/";
		constexpr static std::string_view SeqExposingAll = "*";
		constexpr static std::string_view SeqParenthesisOpen = "(";
		constexpr static std::string_view SeqParenthesisClose = ")";
		constexpr static std::string_view SeqBracketsOpen = "[";
		constexpr static std::string_view SeqBracketsClose = "]";
		constexpr static std::string_view SeqBracesOpen = "{";
		constexpr static std::string_view SeqBracesClose = "}";
		constexpr static std::string_view SeqComma = ",";
		constexpr static std::string_view SeqColon = ":";
		constexpr static std::string_view SeqEqual = "=";
		constexpr static std::string_view SeqBar = "|";
		constexpr static std::string_view SeqUnderscore = "_";
		constexpr static std::string_view SeqDot = ".";
		constexpr static std::string_view SeqMinus = "-";
		constexpr static std::string_view SeqLambda = "\\(";
		constexpr static std::string_view SeqLambdaBind = "->";
		constexpr static std::string_view SeqCaseBind = "->";
		constexpr static std::string_view SeqInfixChars = "!#$%&*+-/:;<=>?^|~`";

		constexpr static std::string_view SmbNewLine = "\n";
		constexpr static std::string_view SmbQuoteString = "\"";
		constexpr static std::string_view SmbQuoteChar = "'";
		constexpr static std::string_view SmbEscape = "\\";

		// 	inline static Parser<void> SkipOneLineComment(ParserFactory& pf) {
		// 		return pf.All<void>(
		// 			pf.Discard<void, void>(),
		// 			pf.Exact(SeqComment),
		// 			pf.Some([](std::string_view const&, char32_t c) { return c != '\n'; })
		// 		);
		// 	}

		// 	inline static Parser<void> SkipMultilineComment(ParserFactory& pf) {
		// 		return pf.All<void>(
		// 			pf.Discard<void, void>(),
		// 			pf.Exact(SeqCommentStart),
		// 			pf.Some([](std::string_view const& v, char32_t c) { return !v.ends_with(SeqCommentEnd); })
		// 		);
		// 	}

		// 	inline static Parser<void> SkipComment(ParserFactory& pf) {
		// 		return pf.OneOf<void>(SkipOneLineComment(pf), SkipMultilineComment(pf), pf.NoopSuccess());
		// 	}

		// 	inline static File ConstructFile(
		// 		Result<void>,
		// 		Result<void>,
		// 		Result<Token> moduleName,
		// 		Result<void>,
		// 		Result<void>
		// 	) {}

		// 	inline static Parser<Token> QualifiedIdentifier(ParserFactory& pf) {
		// 		return pf.Some<Token>(
		// 			[](Token const& token) { return Result<Token>(token); },
		// 			[](std::string_view const& acc, char32_t next) {
		// 				return std::isalnum(static_cast<char>(next), std::locale::classic()) || next == '.';
		// 			}
		// 		);
		// 	}

		// public:
		// 	inline static Parser<File> Create(ParserFactory& pf) {
		// 		return pf.All<File>(
		// 			ConstructFile,
		// 			SkipComment(pf),
		// 			pf.Exact(KwModule),
		// 			QualifiedIdentifier(pf),
		// 			SkipComment(pf),
		// 			pf.ExactEof()
		// 		);
		// 	}
	};
}
