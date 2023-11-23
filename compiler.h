#ifndef LISP_COMPILER_H
#define LISP_COMPILER_H

#include "common.h"
#include "bytecode.h"
#include "scanner.h"
#ifdef DEBUG_BYTECODE_ERRORS
#include "debug.h"
#endif
#include <string>

// Considering splitting this into a parser and compiler
class Compiler {
private:
	Scanner scanner; // Encapsulates source - probably don't need to work with it directly
	Chunk bytecode;
	Token parse_current;  // Note: this is probably still important because '(' can mean different things.
	Token parse_previous;
	bool had_error = false;
	bool panic_mode = false;
	void advance();
	void consume(token_type expected, std::string error_message);
	void number();
	void constant(Value value);
	void expression();
	void error(std::string error_message, Token token);
	void write(uint8_t op);
	void parse();  // Possible to simplify this a lot for a lisp
	void temp_add();
	Chunk& current_bytecode();
public:
	Compiler(std::string& source, Chunk bytecode) : scanner{Scanner(source)}, bytecode{bytecode} {};
	bool compile();
	Chunk get_bytecode() { return bytecode; };
};

#endif