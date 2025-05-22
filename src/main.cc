#include "_external.hh"
#include "nar/parser_package.hh"
#include "parser.hh"

int main(int argc, char const* argv[]) {
	using namespace funcc::parser;
	funcc::nar::PackageParser p{};
	std::shared_ptr<ITokenValue> fileResult = p.ParseFile("tmp/Nar.Base-main/src/List.nar");
	if (fileResult->HasError()) {
		std::shared_ptr<ErrorValue> error = std::dynamic_pointer_cast<ErrorValue>(fileResult);
		std::cerr
			<< "Error: \n"
			<< "tmp/Nar.Base-main/src/List.nar" << ":" << error->GetRange().start.line << ":"
			<< error->GetRange().start.column << "  " << error->GetMessage() << std::endl;
		exit(1);
	}
	std::cout << "Parsed successfully!" << std::endl;
	exit(0);
}
