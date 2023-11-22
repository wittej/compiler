#ifndef LISP_COMPILER_H
#define LISP_COMPILER_H

#include "common.h"
#include "bytecode.h"
#include "scanner.h"
#include <string>

// Considering splitting this into a parser and compiler
class Compiler {
private:
	std::string source;
	Scanner scanner; // Might encapsulate source
	Chunk bytecode;
	Token parse_current;
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
	Chunk& current_bytecode();
public:
	Compiler(std::string source, Chunk bytecode) : source{ source }, scanner{Scanner(source)}, bytecode{bytecode} {};
	bool compile();
	Chunk get_bytecode() { return bytecode; };
};

#endif