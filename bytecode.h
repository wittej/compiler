#ifndef LISP_BYTECODE_H
#define LISP_BYTECODE_H

#include "common.h"
#include "value.h"

enum opcode : uint8_t{
	CONSTANT,
	RETURN,
};

struct Chunk {
	size_t line;
	std::vector<uint8_t> code;
	std::vector<Value> constants;
	std::vector<bool> newlines;
	Chunk(size_t line) : line{ line } {};
	uint8_t add_constant(Value constant);
	void write(uint8_t op, bool newline);
};

#endif