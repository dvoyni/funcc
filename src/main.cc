#include <iostream>

#include "nar/package_parser.hh"

int main(int argc, char const* argv[]) {
	funcc::nar::PackageParser parser{};
	std::shared_ptr<ITokenValue> fileResult = parser.ParseFile("tmp/Nar.Base-main/src/List.nar");
	if (fileResult->GetKind() == ValueKind::Error) {
		std::cerr << "Error: " << std::static_pointer_cast<ErrorValue>(fileResult)->GetMessage() << std::endl;
		exit(1);
	}
	std::cout << "Parsed successfully!" << std::endl;
	exit(0);
}
