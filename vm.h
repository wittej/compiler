#ifndef LISP_VM_H
#define LISP_VM_H

#include "bytecode.h"

enum interpret_result {
	OK,
	COMPILE_ERROR,
	RUNTIME_ERROR,
};

class VirtualMachine {
private:
	interpret_result run(Chunk& bytecode);
public:
	interpret_result interpret(Chunk& bytecode);
};

#endif