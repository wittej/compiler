#include "compiler.h"
#include "scanner.h"


bool
Compiler::compile()
{
	advance();
	while (parse.current.type != token_type::END) {
		definition_or_expression();
		if (parse.current.type != token_type::END) write(opcode::POP);
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
	parse.previous = parse.current;
	for (;;) {
		parse.current = scanner.scan();
		if (parse.current.type != token_type::ERROR) break;
		error("Unrecognized token " + parse.current.string, parse.current);
	}
}

void
Compiler::consume(token_type expected, std::string error_message)
{
	if (parse.current.type == expected) advance();
	else error(error_message, parse.current);
}

void
Compiler::error(std::string error_message, Token token)
{
	if (panic_mode) return;
	std::cerr << "Compiler error [line " << token.line << "] " << error_message << '\n';
	std::cerr << token.string << '\n';
	had_error = true;
	panic_mode = true;
}

// TODO: synchronize

void
Compiler::write(uint8_t op)
{
	current_bytecode().write(op, parse.previous.line);
}

void
Compiler::write_uint(uint16_t uint)
{
	uint8_t constant = static_cast<uint8_t>(uint & 255);
	uint8_t overflow = static_cast<uint8_t>(uint >> 8);
	write(constant);
	write(overflow);
}

size_t
Compiler::write_jump(opcode::opcode jump)
{
	write(jump);
	write(255);  // Will be overwritten by patch_jump
	write(255);
	return current_bytecode().instructions.size() - 2;
}

void
Compiler::patch_jump(size_t jump_index)
{
	int jump = current_bytecode().instructions.size() - jump_index - 2;
	
	// error if > uint16t max

	uint16_t uint16_offset = static_cast<uint16_t>(jump);

	uint8_t offset = static_cast<uint8_t>(uint16_offset & 255);
	uint8_t overflow = static_cast<uint8_t>(uint16_offset >> 8);

	current_bytecode().replace(jump_index, offset);
	current_bytecode().replace(jump_index + 1, overflow);
}

// This will be expanded later for lambdas etc.
Chunk&
Compiler::current_bytecode()
{
	return function->bytecode;
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
	constant(std::stod(parse.previous.string));
}

// TODO: consider making this a built-in function
void
Compiler::temp_not()
{
	parse_next();
	write(opcode::NOT);
}

// Implements short-circuit logic
void
Compiler::temp_and()
{
	parse_next();
	size_t jump_to_exit = write_jump(opcode::JUMP_IF_FALSE);
	write(opcode::POP);
	parse_next();  // TODO: verify this is boolean?
	patch_jump(jump_to_exit);
}

// Implements short-circuit logic
void
Compiler::temp_or()
{
	parse_next();
	size_t jump_to_second = write_jump(opcode::JUMP_IF_FALSE);
	size_t jump_to_exit = write_jump(opcode::JUMP);
	patch_jump(jump_to_second);
	write(opcode::POP);
	parse_next();  // TODO: verify this is boolean?
	patch_jump(jump_to_exit);
}

void
Compiler::definition_or_expression()
{
	expression();
	// TODO: resolve thunks here?
	// TODO: synchronize after error
}

// TODO: differentiate global / local
// TODO: consider simplifying scope depth - no block scope
void
Compiler::definition()
{
	// TODO: differentiate define / let syntax?
	consume(token_type::SYMBOL, "Expect symbol.");

	// TODO: need stack frames for let so it can be used in expressions
	if (scope_depth > 0) {
		for (int i = locals.size() - 1; i >= 0; i--) {
			if (locals[i].depth < scope_depth) break;
			if (parse.previous.string == locals[i].token.string)
				error("Unexpected variable redefinition", parse.previous);
		}
		// Depth -1 indicates variable hasn't been initialized in this scope.
		// TODO: don't do this for lambdas - should be able to be recursive
		Local local{ .token = parse.previous, .depth = -1 /*uninitialized*/ };
		locals.push_back(local);
		expression();  // Needs to be tested
		locals.back().depth = scope_depth;
	}
	else {
		size_t index = vm.global(parse.previous.string);
		expression();
		write(opcode::DEFINE_GLOBAL);
		write_uint(index);
	}
}

// TODO: figure out how to do this
// Notes: there's a lot of potentially messy shared state here - need to
// make sure solution is rational and handles various edge cases.
// Transitional state - secondary compiler.
// Medium-term - make this a method? Scope struct?

void
Compiler::lambda()
{
	Compiler compiler(this);
	compiler.parse = parse;

	compiler.consume(token_type::LPAREN, "Expected '(' before function parameters");
	while (compiler.parse.current.type != token_type::RPAREN) {
		compiler.consume(token_type::SYMBOL, "Expect symbol parameter");
		compiler.locals.push_back(Local{.token = compiler.parse.previous,
			.depth=static_cast<int>(compiler.scope_depth)});
		compiler.function->arity++;
	}
	compiler.consume(token_type::RPAREN, "Expected ')' after function parameters");
	while (compiler.parse.current.type != token_type::RPAREN && compiler.parse.current.type != token_type::END) {
		compiler.definition_or_expression();
		if (compiler.parse.current.type != token_type::RPAREN && compiler.parse.current.type != token_type::END)
			compiler.write(opcode::POP);
	}
	compiler.write(opcode::RETURN);

	parse = compiler.parse;
	write(opcode::CLOSURE);
	constant(vm.allocate(compiler.function));

	// Will need to obtain function upvalue count
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

int
Compiler::resolve_upvalue(Token token)
{
	if (this->enclosing == nullptr) return -1;

	int local = this->enclosing->resolve_local(token);
	if (local >= 0) {
		push_upvalue(local, true);
	}

	int upvalue = this->enclosing->resolve_upvalue(token);
	if (upvalue >= 0) {
		return push_upvalue(upvalue, false);		
	}

	return -1;
}

int
Compiler::push_upvalue(int index, bool local)
{
	if (index < 0) return -1;

	for (size_t i = 0; i < upvalues.size(); i++) {
		if (upvalues[i].index == index && upvalues[i].is_local) return i;
	}

	// TODO: function?
	upvalues.push_back(Upvalue{
		.index = static_cast<size_t>(index),
		.is_local = local
		});
	// Note: function will need to know how many upvalues it has?
	return static_cast<int>(upvalues.size() - 1);
}

void
Compiler::symbol()
{
	int local = resolve_local(parse.previous);
	if (local >= 0) {
		size_t index = local;
		write(opcode::GET_LOCAL);
		write_uint(index);
	}

	else if ((local = resolve_upvalue(parse.previous)) != -1) {
		size_t index = local;
		write(opcode::GET_UPVALUE);
		write_uint(index);
	}
	
	else {
		size_t index = vm.global(parse.previous.string);
		write(opcode::GET_GLOBAL);
		write_uint(index);
	}
}

// TODO: is this needed?
void
Compiler::expression()
{
	// TODO - differentiate self-evaluating and combination?
	parse_next();
}

// TODO: implement as special case of lambda?
void
Compiler::temp_let()
{
	consume(token_type::LPAREN, "Expect '('.");
	++scope_depth;
	// TODO: local definitions
	while (parse.current.type == token_type::LPAREN) {
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
Compiler::if_statement()
{
	expression();  // predicate - value will be popped in either branch	
	size_t jump_to_alternative = write_jump(opcode::JUMP_IF_FALSE);

	write(opcode::POP);
	expression();  // consequent
	size_t jump_to_exit = write_jump(opcode::JUMP);

	patch_jump(jump_to_alternative);
	write(opcode::POP);
	expression();  // alternative

	patch_jump(jump_to_exit);
}

void
Compiler::call()
{
	uint16_t number_arguments = 0;
	while (parse.current.type != token_type::RPAREN) {
		// TODO: want a function body that evaluates thunks
		if (parse.current.type == token_type::END) {  // TEMP
			error("Expected closing ')'", parse.current);
			break;
		}
		expression();
		number_arguments++;
	}
	write(opcode::CALL);
	write_uint(number_arguments);
}

void
Compiler::parse_next()
{
	advance();
	switch (parse.previous.type) {
	case token_type::NUMBER:
		number();
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
	case token_type::LPAREN:
		if (parse.current.type == token_type::RPAREN) write(opcode::NIL);
		else combination();
		consume(token_type::RPAREN, "Expect ')'.");
		break;
	default:
		error("unknown self-evaluating token type.", parse.previous);
	}
}

void
Compiler::combination()
{
	advance();
	switch (parse.previous.type) {
	case token_type::NOT:
		temp_not();
		break;
	case token_type::AND:
		temp_and();
		break;
	case token_type::OR:
		temp_or();
		break;
	case token_type::LET:
		temp_let();
		break;
	case token_type::DEFINE:
		definition();
		write(opcode::NIL);  // TODO: consider moving this to definition
		break;
	case token_type::LAMBDA:
		lambda();
		break;
	case token_type::IF:
		if_statement();
		break;
	case token_type::SYMBOL:
		symbol();
		call();
		break;
	default:
		error("expected symbol or built-in when reading combination.", parse.previous);
	}
}