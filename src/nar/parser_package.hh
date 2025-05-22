#pragma once

#include "../_external.hh"
#include "ast_common.hh"
#include "parser_file.hh"

namespace funcc::nar {
	class PackageParser {
	public:
		std::shared_ptr<ITokenValue> ParseFile(std::string const& filePath) {
			std::ifstream fileStream{filePath};
			if (!fileStream.is_open()) {
				return std::make_shared<ErrorValue>(std::string("Failed to open file ") + filePath);
			}

			std::string fileContent{std::istreambuf_iterator<char>(fileStream), std::istreambuf_iterator<char>()};
			Utf8Reader reader{fileContent};

			return FileParser::PFile->Consume(reader);
		}
	};
}
