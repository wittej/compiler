#include "compiler.h"
#include "scanner.h"


void
compile(std::string source)
{
	Scanner scanner(source);

	int line = -1;
	for (;;) {
		Token token = scanner.scan();

		if (token.line != line) {
			std::cout << std::setfill(' ') << std::setw(4) << token.line << ' ';
			line = token.line;
		}
		else {
			std::cout << "   | ";
		}

		std::cout << token.type << ' ' << token.string << '\n';

		if (token.type == token_type::END) break;
	}
}