#ifndef LISP_BYTECODE_H
#define LISP_BYTECODE_H

#include "common.h"
#include "value.h"


enum opcode : uint8_t {
	CONSTANT,
	CONSTANT_LONG,
	ADD,
	RETURN,
};

struct ConstantIndex {
	uint8_t index;
	uint8_t overflow;
};

struct Chunk {
private:
	size_t max_line;
public:
	const size_t base_line;
	std::vector<uint8_t> instructions;
	std::vector<Value> constants;
	std::vector<bool> newlines;
	Chunk(size_t line) : base_line{ line }, max_line{ line } {};
	ConstantIndex add_constant(Value constant);
	void write(uint8_t op, size_t line);
};

#endif