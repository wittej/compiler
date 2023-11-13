#ifndef LISP_VM_H
#define LISP_VM_H

#include "bytecode.h"

#define STACK_MAX 256

enum interpret_result {
	OK,
	COMPILE_ERROR,
	RUNTIME_ERROR,
};

class VirtualMachine {
private:
	interpret_result run(Chunk& bytecode);
	std::vector<Value> stack;
	Value stack_pop();
public:
	interpret_result interpret(Chunk& bytecode);
};

#endif