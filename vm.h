#ifndef LISP_VM_H
#define LISP_VM_H

#include "bytecode.h"

enum interpret_result {
	OK,
	COMPILE_ERROR,
	RUNTIME_ERROR,
};

// TODO: this probably wants to be a class
struct VirtualMachine {
	Chunk& chunk;
	uint8_t *ip;
	interpret_result interpret(Chunk& bytecode);
};

#endif