#ifndef LISP_BYTECODE_H
#define LISP_BYTECODE_H

#include "common.h"
#include "value.h"

enum opcode : uint8_t {
	CONSTANT,
	RETURN,
};

typedef struct {
	std::vector<uint8_t> code;
	std::vector<Value> constants;
} Chunk;

uint8_t add_constant(Chunk& chunk, Value constant);

#endif