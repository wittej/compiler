#include "compiler.h"
#include "scanner.h"


bool
Compiler::compile()
{
	advance();
	while (parse_current.type != token_type::END) {
		definition_or_expression();
	}
	write(opcode::RETURN);
#ifdef DEBUG_BYTECODE_ERRORS
	if (!had_error) {
		disassembleBytecode(current_bytecode(), "CODE");
	}
#endif
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
	if (parse_current.type == expected) advance();
	else error(error_message, parse_current);
}

void
Compiler::error(std::string error_message, Token token)
{
	if (panic_mode) return;
	std::cerr << "Compiler error [line " << token.line << "] " << error_message << "\n";
	had_error = true;
	panic_mode = true;
}

// TODO: synchronize

void
Compiler::write(uint8_t op)
{
	current_bytecode().write(op, parse_previous.line);
}

// This will be expanded later for lambdas etc.
Chunk&
Compiler::current_bytecode()
{
	return bytecode;
}

// This needs to get tested for long constants.
void
Compiler::constant(Value value)
{
	ConstantIndex c = current_bytecode().add_constant(value);
	write(c.overflow == 0 ? opcode::CONSTANT : opcode::CONSTANT_LONG);
	// Endianness matters here - this seems right but should double-check.
	write(c.index);
	if (c.overflow != 0) write(c.overflow);
}

void
Compiler::number()
{
	constant(std::stod(parse_previous.string));
}

// Temporary - just to learn and make sure compiler is working.
void
Compiler::temp_add()
{
	parse();
	parse();
	write(opcode::ADD);
}

void
Compiler::temp_equal()
{
	parse();
	parse();
	write(opcode::EQUAL);
}

void
Compiler::temp_not()
{
	parse();
	write(opcode::NOT);
}

void
Compiler::temp_cons()
{
	parse();
	parse();
	write(opcode::CONS);
}

void
Compiler::definition_or_expression()
{
	if (parse_current.type == token_type::DEFINE) {
		definition();
	}
	else {
		expression();
	}
	//TODO: synchronize after error
}

void
Compiler::definition()
{
	advance();  // TEMP - implement match
	consume(token_type::SYMBOL, "Expect symbol.");
	double index = vm.global(parse_previous.string);  // TOOD: consider dedicated uint Value

	expression();

	constant(Value(index));
	write(opcode::DEFINE_GLOBAL);
}

void
Compiler::symbol()
{
	double index = vm.global(parse_previous.string);  // TODO: consider dedicated uint Value
	constant(Value(index));
	write(opcode::GET_GLOBAL);
}

void
Compiler::expression()
{
	parse();
}

void
Compiler::parse()
{
	advance();
	switch (parse_previous.type) {
	case token_type::NUMBER:
		number();
		break;
	case token_type::PLUS:
		temp_add();
		break;
	case token_type::NOT:
		temp_not();
		break;
	case token_type::EQUAL:
		temp_equal();
		break;
	case token_type::CONS:
		temp_cons();
		break;
	case token_type::SYMBOL:
		symbol();
		break;
	case token_type::FALSE:
		write(opcode::FALSE);
		break;
	case token_type::TRUE:
		write(opcode::TRUE);
		break;
	case token_type::NIL:
		write(opcode::NIL);
		break;
	default:
		error("unknown token type.", parse_previous);
	}
}