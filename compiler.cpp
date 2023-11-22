#include "compiler.h"
#include "scanner.h"


bool
Compiler::compile()
{
	advance();
	expression();
	consume(token_type::END, "Expect end of expression.");
}

void
Compiler::advance()
{
	parse_previous = parse_current;
	for (;;) {
		parse_current = scanner.scan();
		if (parse_current.type != token_type::ERROR) break;
		error(parse_current.string, parse_current);
	}
}

