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
	void advance();
	void consume(token_type expect, std::string error);
	void expression();
	void error(std::string error, Token token);
public:
	Compiler(std::string source, Chunk bytecode) : source{ source }, scanner{Scanner(source)}, bytecode{bytecode} {};
	bool compile();
	Chunk get_bytecode();
};

#endif