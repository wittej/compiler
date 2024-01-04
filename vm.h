#ifndef LISP_VM_H
#define LISP_VM_H

#include "common.h"
#include "bytecode.h"
#include "value.h"
#include "debug.h"

#define STACK_MAX 256

enum class interpret_result {
	OK,
	COMPILE_ERROR,
	RUNTIME_ERROR,
};

class VirtualMachine {
private:
	interpret_result run(Chunk& bytecode);
	std::vector<Value> stack;
	std::forward_list<Data> memory;
	// TODO: benchmark vector vs. map performance here
	// Hybrid solution - string-index map, value vector globals
	std::unordered_map<std::string, size_t> global_indexes;
	std::vector<Value> globals;
	Value stack_pop();
	Value stack_peek(size_t depth);
	void runtime_error(std::string message, size_t line);
	bool truthValue(Value val);
public:
	interpret_result interpret(std::string source);
	Value allocate(std::string string);
	size_t global(std::string key);
};

#endif