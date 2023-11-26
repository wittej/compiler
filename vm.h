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
	Value stack_peek(size_t depth);
	void runtime_error(std::string message, size_t line);
	bool truthValue(Value val);
public:
	interpret_result interpret(std::string source);
};

#endif