#ifndef LISP_BYTECODE_H
#define LISP_BYTECODE_H

#include "common.h"
#include "value.h"

enum class opcode {
	RETURN,
};

typedef struct {
	std::vector<opcode> code;
	std::vector<Value> constants;
} Chunk;

#endif