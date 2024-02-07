#ifndef LISP_VM_H
#define LISP_VM_H

#include "common.h"
#include "bytecode.h"
#include "value.h"
#include "debug.h"

#define RECURSION_MAX 64
#define STACK_MAX 256 * RECURSION_MAX

enum class interpret_result {
	OK,
	COMPILE_ERROR,
	RUNTIME_ERROR,
};

struct CallFrame {
	std::shared_ptr<Function> function;  // TODO: determine shared ptr or reference
	uint8_t* ip;
	size_t stack_index;
};

class VirtualMachine {
private:
	interpret_result run();
	std::vector<Value> stack;
	std::vector<CallFrame> frames;
	std::forward_list<Data> memory;
	// TODO: benchmark vector vs. map performance here
	// Hybrid solution - string-index map, value vector globals
	std::unordered_map<std::string, size_t> global_indexes;
	std::vector<Value> globals;
	Value stack_pop();
	Value stack_peek(size_t depth);
	void runtime_error(std::string message, size_t line);
	bool truthValue(Value val);
	inline uint16_t read_uint16_and_update_ip(uint8_t*& ip);
	bool call(size_t number_arguments);
	bool call(std::shared_ptr<Function> function, size_t number_arguments);
	bool call(std::shared_ptr<BuiltinFunction> function, size_t number_arguments);
public:
	interpret_result interpret(std::string source);
	// TODO: template?
	Value allocate(std::string string);
	Value allocate(std::shared_ptr<Function> function);
	Value allocate(Pair pair);
	size_t global(std::string key);
};

#endif