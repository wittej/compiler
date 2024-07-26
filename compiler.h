#ifndef LISP_COMPILER_H
#define LISP_COMPILER_H

#include "common.h"
#include "bytecode.h"
#include "scanner.h"
#include "value.h"
#ifdef DEBUG_BYTECODE_ERRORS
#include "debug.h"
#endif
#include "vm.h"

struct Local {
	Token token;
	int depth;
	bool captured = false;
};

struct Upvalue {
	size_t index;
	bool is_local;
};

struct ParseState {
	Token current;  // Note: this is probably still important because '(' can mean different things.
	Token previous;
};

/**
 * Handles parsing and compilation - gets tokens from the scanner and writes
 * them as bytecode, resolving variables and storing constants as needed.
 * 
 * NB: subject to change - some features of this class are likely best
 * encapsulated in other classes.
 */
// TODO: consider decoupling parsing and compiling jobs
class Compiler {
private:
	Scanner& scanner; // TODO: move to parser?
	ParseState parse;  // TODO: move features to here?
	VirtualMachine& vm;
	
	/* TODO: manage these and access with a stack data structure. */
	std::shared_ptr<Function> function = std::make_shared<Function>();
	Compiler* enclosing = nullptr;  // This needs to be nullable.
	size_t scope_depth = 0;
	std::vector<Local> locals;
	std::vector<Upvalue> upvalues;

	bool had_error = false;  // TODO: clean this up
	bool panic_mode = false;
	void advance();
	void consume(token_type expected, std::string error_message);
	void number();
	void constant(Value value);
	void definition_or_expression();  // TODO: synchronize here
	void definition();
	void expression();
	void lambda();
	void _if();
	void error(std::string error_message, Token token);
	void write(uint8_t op);
	void parse_next();  // TODO: infix . for cons - excuse to practice parser design
	void combination();
	void _not();
	void _and();
	void _or();
	void symbol();
	void call();
	void write_uint16(uint16_t uint);
	size_t write_jump(uint8_t jump);
	void patch_jump(size_t patch_index);
	int resolve_local(Token token);
	int resolve_upvalue(Token token);
	int push_upvalue(int index, bool local);
	Chunk& current_bytecode();
public:
	Compiler(Scanner& scanner, VirtualMachine& vm) : scanner{ scanner }, vm{ vm } {};
	Compiler(Compiler* enclosing) : enclosing{ enclosing }, scanner{ enclosing->scanner },
		vm{ enclosing->vm }, scope_depth{enclosing->scope_depth + 1} {};
	bool error() { return had_error; };  // TODO: needed?
	std::shared_ptr<Function> compile();
};

#endif