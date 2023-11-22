#include "compiler.h"
#include "scanner.h"


bool
Compiler::compile()
{
	advance();
	expression();
	consume(token_type::END, "Expect end of expression.");
	return !had_error;
}

void
Compiler::advance()
{
	parse_previous = parse_current;
	for (;;) {
		parse_current = scanner.scan();
		if (parse_current.type != token_type::ERROR) break;
		error("Unrecognized token " + parse_current.string, parse_current);
	}
}

void
Compiler::consume(token_type expected, std::string error_message)
{
	if (parse_current.type == expected) advance;
	else error(error_message, parse_current);
}

void
Compiler::error(std::string error_message, Token token)
{
	if (panic_mode) return;
	std::cerr << "Error [line " << token.line << "] " << error_message << "\n";
	had_error = true;
	panic_mode = true;
}