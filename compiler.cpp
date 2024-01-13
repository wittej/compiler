#include "compiler.h"
#include "scanner.h"


bool
Compiler::compile()
{
	advance();
	while (parse_current.type != token_type::END) {
		// TODO: pop values that aren't returned to REPL
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

void
Compiler::write_uint(uint16_t uint)
{
	uint8_t constant = static_cast<uint8_t>(uint & 255);
	uint8_t overflow = static_cast<uint8_t>(uint >> 8);
	write(constant);
	write(overflow);
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
	expression();
	// TODO: resolve thunks here?
	// TODO: synchronize after error
}

// TODO: differentiate global / local
void
Compiler::definition()
{
	// TODO: differentiate define / let syntax?
	consume(token_type::SYMBOL, "Expect symbol.");

	// TODO: need stack frames for let so it can be used in expressions
	if (scope_depth > 0) {
		for (int i = locals.size() - 1; i >= 0; i--) {
			if (locals[i].depth < scope_depth) break;
			if (parse_previous.string == locals[i].token.string)
				error("Unexpected variable redefinition", parse_previous);
		}
		// Depth -1 indicates variable hasn't been initialized in this scope.
		Local local{ .token = parse_previous, .depth = -1 /*uninitialized*/ };
		locals.push_back(local);
		expression();  // Needs to be tested
		locals.back().depth = scope_depth;
	}
	else {
		size_t index = vm.global(parse_previous.string);
		expression();
		write(opcode::DEFINE_GLOBAL);
		write_uint(index);
	}
}

int
Compiler::resolve_local(Token token)
{
	for (int i = locals.size() - 1; i >= 0; i--) {
		// Intended behavior: get from closest initialized scope.
		if (token.string == locals[i].token.string && locals[i].depth >= 0)
			return i;
	}
	return -1;
}

void
Compiler::symbol()
{
	int local = resolve_local(parse_previous);
	if (local >= 0) {
		size_t index = local;
		write(opcode::GET_LOCAL);
		write_uint(index);
	}
	else {
		size_t index = vm.global(parse_previous.string);
		write(opcode::GET_GLOBAL);
		write_uint(index);
	}
}

// TODO: is this needed?
void
Compiler::expression()
{
	parse();
}

void
Compiler::temp_let()
{
	consume(token_type::LPAREN, "Expect '('.");
	++scope_depth;
	// TODO: local definitions
	while (parse_current.type == token_type::LPAREN) {
		advance();
		definition();
		consume(token_type::RPAREN, "Expect ')'.");
	}
	
	consume(token_type::RPAREN, "Expect ')'.");
	expression();  // NB: should be something like <definition>* <expression>* <expression>
	write(opcode::POP);  // TEMP - throw away the result of that expression
	// TODO: do this only for expressions that aren't the final result.

	--scope_depth;
	while (locals.size() > 0 && locals.back().depth > scope_depth) {
		write(opcode::POP);
		locals.pop_back();
	}

	write(opcode::NIL);  // TODO: replace with a return procedure
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
	case token_type::LET:
		temp_let();
		break;
	case token_type::DEFINE:
		definition();
		write(opcode::NIL);  // TODO: consider moving this to definition
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
	// NOTE: this will need to be smarter - parse correct number of args etc
	case token_type::LPAREN:
		expression();
		consume(token_type::RPAREN, "Expect ')'.");
		break;
	default:
		error("unknown token type.", parse_previous);
	}
}