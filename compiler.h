#ifndef LISP_COMPILER_H
#define LISP_COMPILER_H

#include "common.h"
#include "bytecode.h"
#include "scanner.h"
#ifdef DEBUG_BYTECODE_ERRORS
#include "debug.h"
#endif
#include "vm.h"

struct Local {
	Token token;
	size_t depth;
};

// Considering splitting this into a parser and compiler
class Compiler {
private:
	Scanner scanner; // Encapsulates source - probably don't need to work with it directly
	VirtualMachine& vm;
	Chunk bytecode;
	Token parse_current;  // Note: this is probably still important because '(' can mean different things.
	Token parse_previous;
	size_t local_scope = 0;
	std::vector<Local> locals;
	bool had_error = false;
	bool panic_mode = false;
	void advance();
	void consume(token_type expected, std::string error_message);
	void number();
	void constant(Value value);
	void definition_or_expression();  // TODO: synchronize here
	void definition();
	void expression();
	void error(std::string error_message, Token token);
	void write(uint8_t op);
	void parse();  // TODO: infix . for cons - excuse to practice parser design
	void temp_add();
	void temp_not();
	void temp_equal();
	void temp_cons();
	void temp_let();
	void symbol();
	Chunk& current_bytecode();
public:
	Compiler(std::string& source, VirtualMachine& vm, Chunk bytecode) : scanner{ Scanner(source) }, vm{ vm }, bytecode{ bytecode } {};
	bool compile();
	Chunk get_bytecode() { return bytecode; };
};

#endif