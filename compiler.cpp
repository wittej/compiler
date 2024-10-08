#include "compiler.h"
#include "scanner.h"

/* Future design note - definition_or_expression is a legacy of when this was
 * a more Scheme-like Lisp dialect. It's similar to expression() and
 * parse_next(), and these should be refactored. I'm moving to definitions
 * being expressions that return NIL. */


// TODO: refactor into stack
/**
 * Compile the source and return the top-level function.
 * 
 * @return: shared pointer to top-level function.
 */
std::shared_ptr<Function>
Compiler::compile()
{
	locals.push_back(Local{
		.token = Token{.type = token_type::BEGIN, .line = 0 },
		.depth = 0});

	advance();
	while (scanner.current.type != token_type::END) {
		definition_or_expression();
		if (scanner.current.type != token_type::END) write(opcode::POP);
	}
	write(opcode::RETURN);
#ifdef DEBUG_BYTECODE_ERRORS
	if (!had_error) {
		disassembleBytecode(function->bytecode, "CODE");
	}
#endif
	return had_error ? nullptr : function;
}

/**
 * Move forward one token. Retain previous token in case it is needed to parse
 * current token. Contains under-construction error strategy - idea is to
 * support recovery to the next non-error statement if possible.
 */
void
Compiler::advance()
{
	// TODO: reset panic mode at next definition or expression?
	for (;;) {  // If an error is encountered, go to next non-error token.
		scanner.advance();
		if (scanner.current.type != token_type::ERROR) break;
		error("Unrecognized token " + scanner.current.string, scanner.current);
	}
}

/**
 * Advance one token if it is what is expected or indicate an error otherwise.
 * 
 * @param expected: next token type required.
 * @param error_message: error message if token is not expected type.
 */
void
Compiler::consume(token_type expected, std::string error_message)
{
	if (scanner.current.type == expected) advance();
	else error(error_message, scanner.current);
}

/**
 * Display error to user and indicate to compiler that program is invalid,
 * preventing the program from being run by the VM.
 * 
 * @param error_message: message to display to user.
 * @param token: token where error was encountered.
 */
void
Compiler::error(std::string error_message, Token token)
{
	if (panic_mode) return;
	std::cerr << "Compiler error [line " << token.line << "] " << error_message << " ";
	std::cerr << token.string << '\n';
	had_error = true;
	panic_mode = true;
}

// TODO: synchronize

/**
 * Write bytecode for the specified uint8.
 * 
 * @param op: opcode or other uint8 to write.
 */
void
Compiler::write(uint8_t op)
{
	function->bytecode.write(op, scanner.previous.line);
}

/**
 * Write bytecode for the specified uint16 as two uint8 in little-endian order.
 *
 * @param uint: uint16 to write in little-endian order.
 */
void
Compiler::write_uint16(uint16_t uint)
{
	uint8_t constant = static_cast<uint8_t>(uint & 255);
	uint8_t overflow = static_cast<uint8_t>(uint >> 8);
	write(constant);
	write(overflow);
}

/**
 * Used with patch_jump - this writes the jump instruction, reserves space for
 * the constant part of the instruction, and returns index of the constant.
 * 
 * @param jump: jump instruction to write.
 * @return: index of the constant part of the jump instruction.
 */
size_t
Compiler::write_jump(uint8_t jump)
{
	write(jump); // TODO: verify valid jump opcode?
	write(255);  // Will be overwritten by patch_jump
	write(255);
	return function->bytecode.instructions.size() - 2;
}

/**
 * Update the placeholder bytecode written by write_jump to add jump target.
 * 
 * @param patch_index: first index of the index part of the jump instruction.
 */
void
Compiler::patch_jump(size_t patch_index)
{
	// error if not a valid size_t

	// Number of indices to jump forward (past jump instruction at minimum).
	size_t jump_to = function->bytecode.instructions.size() - patch_index - 2;
	
	// error if > uint16t max

	auto uint16_offset = static_cast<uint16_t>(jump_to);

	auto offset = static_cast<uint8_t>(uint16_offset & 255);
	auto overflow = static_cast<uint8_t>(uint16_offset >> 8);

	// Update integer part of jump instruction (little-endian format).
	function->bytecode.instructions[patch_index] = offset;
	function->bytecode.instructions[patch_index + 1] = overflow;
}

/**
 * Write a constant Value. Updates bytecode constants vector.
 * 
 * @param value: Value of constant.
 */
void
Compiler::constant(Value value)
{
	uint16_t index = function->bytecode.add_constant(value);
	write(opcode::CONSTANT);
	write_uint16(index);
}

// TODO: break out into integer and double versions.
/**
 * Interpret the current token as a decimal and add it as a constant. Updates
 * bytecode constants vector.
 */
void
Compiler::number()
{
	constant(std::stod(scanner.previous.string));
}

// TODO: consider making this a built-in function
/**
 * Implements logical NOT function.
 */
void
Compiler::_not()
{
	parse_next();
	write(opcode::NOT);
}

/**
 * Implements logical AND function. This uses short-circuit logic - second
 * statement will only be evaluated if first does not evaluate to false.
 */
void
Compiler::_and()
{
	parse_next();
	size_t jump_to_exit = write_jump(opcode::JUMP_IF_FALSE);
	write(opcode::POP);
	parse_next();  // TODO: verify this is boolean?
	patch_jump(jump_to_exit);
	// Temp - convert to bool
	write(opcode::NOT);
	write(opcode::NOT);
}

/**
 * Implements logical OR function. This uses short-circuit logic - second
 * statement will only be evaluated if first evaluates to false.
 */
void
Compiler::_or()
{
	parse_next();
	size_t jump_to_second = write_jump(opcode::JUMP_IF_FALSE);
	size_t jump_to_exit = write_jump(opcode::JUMP);
	patch_jump(jump_to_second);
	write(opcode::POP);
	parse_next();  // TODO: verify this is boolean?
	patch_jump(jump_to_exit);
	// Temp - convert to bool
	write(opcode::NOT);
	write(opcode::NOT);
}

// TODO: refactor expression, parse_next, and definition_or_expression.
/**
 * NB: this function is currently a placeholder, but will have a role in thunk
 * resolution when I implement tail recursion and synchronization after error
 * after next compiler output pass.
 */
void
Compiler::definition_or_expression()
{
	expression();
	// TODO: resolve thunks here?
	// TODO: synchronize after error
}

// TODO: revise for let as needed (basically as a lambda)
/**
 * Compile a global or local definition (depending on scope depth). Does not
 * allow variable redefinition.
 */
void
Compiler::definition()
{
	consume(token_type::SYMBOL, "Expect symbol.");

	if (scope_depth > 0) {
		for (int i = locals.size() - 1; i >= 0; i--) {
			if (locals[i].depth < scope_depth) break;
			if (scanner.previous.string == locals[i].token.string)
				error("Unexpected variable redefinition", scanner.previous);
		}

		Local local{ .token = scanner.previous,
					 .depth = static_cast<int>(scope_depth)};
		locals.push_back(local);
		expression();  // Needs to be tested
	}

	else {
		if (vm.check_global(scanner.previous.string)) {
			error("Unexpected variable redefinition", scanner.previous);
			return;
		}

		size_t index = vm.global(scanner.previous.string);
		expression();
		write(opcode::DEFINE_GLOBAL);
		write_uint16(index);
	}
}

void
Compiler::set()
{
	consume(token_type::SYMBOL, "Expect symbol.");

	int local = resolve_local(scanner.previous);
	if (local >= 0) {
		size_t index = local;
		expression();
		write(opcode::SET_LOCAL);
		write_uint16(index);
	}

	else if ((local = resolve_upvalue(scanner.previous)) != -1) {
		size_t index = local;
		expression();
		write(opcode::SET_UPVALUE);
		write_uint16(index);
	}

	else if (vm.check_global(scanner.previous.string)) {
		size_t index = vm.global(scanner.previous.string);
		expression();
		write(opcode::SET_GLOBAL);
		write_uint16(index);
	}

	else {
		error("Attempt to set undefined variable", scanner.previous);
	}
}

// TODO: write a "let" based on this.
// TOOD: non-anonymous functions (similar to this - possible modification).
/**
 * Compile an anonymous function - function body is similar to the top-level
 * script as a whole. Result of final expression is returned.
 */
void
Compiler::lambda()
{
	Compiler compiler(this);

	// TODO: prevent redundant params

	compiler.consume(token_type::LPAREN, "Expected '(' before function parameters");
	while (scanner.current.type != token_type::RPAREN) {
		compiler.consume(token_type::SYMBOL, "Expect symbol parameter");

		for (auto& local : compiler.locals) {
			if (local.token.string == scanner.previous.string) {
				compiler.error("Unexpected duplicate token",
							   scanner.previous);
			}
		}

		compiler.locals.push_back(Local{.token = scanner.previous,
			.depth=static_cast<int>(compiler.scope_depth)});
		compiler.function->arity++;
	}
	compiler.consume(token_type::RPAREN, "Expected ')' after function parameters");
	while (scanner.current.type != token_type::RPAREN && scanner.current.type != token_type::END) {
		compiler.definition_or_expression();
		if (scanner.current.type != token_type::RPAREN && scanner.current.type != token_type::END)
			compiler.write(opcode::POP);
	}
	compiler.write(opcode::RETURN);
	compiler.function->bytecode.tail_call_optimize();

	if (compiler.had_error) error("Error compiling function", scanner.previous);
#ifdef DEBUG_BYTECODE_ERRORS
	else {
		disassembleBytecode(compiler.function->bytecode, "FUNCTION CODE");
	}
#endif

	write(opcode::CLOSURE);
	
	uint16_t index = function->bytecode.add_constant(vm.allocate(compiler.function));
	write_uint16(index);

	compiler.function->upvalues = compiler.upvalues.size();
	for (size_t i = 0; i < compiler.function->upvalues; i++) {
		write(compiler.upvalues[i].is_local ? 1 : 0);
		write_uint16(compiler.upvalues[i].stack_index);
	}
}

/**
 * Return index of a local Value or report if not found.
 * 
 * @param token: Token value of local to search for.
 * @return: index of local (if found) or -1 (otherwise).
 */
int
Compiler::resolve_local(Token token)
{
	// TODO: rewrite with hashmap?
	for (int i = locals.size() - 1; i >= 0; i--) {
		// Intended behavior: get from closest initialized scope.
		if (token.string == locals[i].token.string && locals[i].depth >= 0)
			return i;
	}
	return -1;
}

/**
 * Attempt to locate the stack index associated with this token by walking the
 * scopes recursively. This is slow during compile time but enables fast value
 * lookups during runtime.
 * 
 * @param token: Token value to search for.
 * @return: stack index (if found) or -1 (otherwise).
 */
int
Compiler::resolve_upvalue(Token token)
{
	if (this->enclosing == nullptr) return -1;

	int local = this->enclosing->resolve_local(token);
	if (local >= 0) {
		this->enclosing->locals[local].captured = true;
		return push_upvalue(local, true);
	}

	int upvalue = this->enclosing->resolve_upvalue(token);
	if (upvalue >= 0) {
		return push_upvalue(upvalue, false);		
	}

	return -1;
}

/**
 * Add upvalue if needed (with the corresponding stack index) and return its
 * index in the compiler's vector of upvalues for this scope.
 * 
 * @param stack_index: index in runtime stack associated with this upvalue.
 * @param local: is this value local to this scope?
 * @return: index (in upvalues vector) of this upvalue.
 */
int
Compiler::push_upvalue(int stack_index, bool local)
{
	if (stack_index < 0) return -1;

	for (size_t i = 0; i < upvalues.size(); i++) {
		if (upvalues[i].stack_index == stack_index && upvalues[i].is_local) {
			return i;
		}
	}

	// TODO: function?
	upvalues.push_back(Upvalue{
		.stack_index = static_cast<size_t>(stack_index),
		.is_local = local
		});

	return static_cast<int>(upvalues.size() - 1);
}

/**
 * Compile a reference to a symbol, starting with the current scope and
 * proceeding to the global scope. Use deepest scope depth if symbol has been
 * shadowed.
 */
void
Compiler::symbol()
{
	int local = resolve_local(scanner.previous);
	if (local >= 0) {
		size_t index = local;
		write(opcode::GET_LOCAL);
		write_uint16(index);
	}

	else if ((local = resolve_upvalue(scanner.previous)) != -1) {
		size_t index = local;
		write(opcode::GET_UPVALUE);
		write_uint16(index);
	}
	
	else {
		size_t index = vm.global(scanner.previous.string);
		write(opcode::GET_GLOBAL);
		write_uint16(index);
	}
}

// TODO: refactor expression, parse_next, and definition_or_expression.
void
Compiler::expression()
{
	parse_next();
}

/**
 * Implements a Lisp-style ternary if with short-circuit evaluation.
 */
void
Compiler::_if()
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

/**
 * Compile a call to a built-in or user-defined function / combination.
 */
void
Compiler::call()
{
	uint16_t number_arguments = 0;
	while (scanner.current.type != token_type::RPAREN) {
		// TODO: consider role of thunk evaluation here.
		if (scanner.current.type == token_type::END) {  // EOF reached early
			error("Expected closing ')'", scanner.current);
			break;
		}
		expression();
		number_arguments++;
	}
	write(opcode::CALL);
	write_uint16(number_arguments);
}

// TODO: refactor expression, parse_next, and definition_or_expression.
/**
 * Parse and compile the next expression - may be self-evaluating or a
 * combination.
 */
void
Compiler::parse_next()
{
	advance();
	switch (scanner.previous.type) {
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
			if (scanner.current.type == token_type::RPAREN) write(opcode::NIL);
			else combination();
			consume(token_type::RPAREN, "Expect ')'.");
			break;
		default:
			error("unknown self-evaluating token type", scanner.previous);
	}
}

/**
 * Parse and compile a combination - special forms are handled using specific
 * tokens, and other built-ins and user-defined functions are handled
 * generally. '()' for 'NIL' is supported but is handled elsewhere.
 */
void
Compiler::combination()
{
	advance();
	switch (scanner.previous.type) {
		case token_type::NOT:
			_not();
			break;
		case token_type::AND:
			_and();
			break;
		case token_type::OR:
			_or();
			break;
		case token_type::DEFINE:
			definition();
			write(opcode::NIL);  // TODO: consider moving this to definition
			break;
		case token_type::SET:
			set();
			write(opcode::NIL);
			break;
		case token_type::LAMBDA:
			lambda();
			break;
		case token_type::IF:
			_if();
			break;
		case token_type::SYMBOL: // It's a (non special form) function call.
			symbol();
			call();
			break;
		default:
			error("expected symbol when reading combination", scanner.previous);
	}
}