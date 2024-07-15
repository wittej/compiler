#ifndef LISP_VM_H
#define LISP_VM_H

#include "common.h"
#include "bytecode.h"
#include "value.h"
#include "debug.h"
#include "function.h"

// TODO: constexpr
#define RECURSION_MAX 64
#define STACK_MAX 256 * RECURSION_MAX

// TODO: correct return process for blank line

enum class interpret_result {
	OK,
	COMPILE_ERROR,
	RUNTIME_ERROR,
};

struct CallFrame {
	std::shared_ptr<Closure> closure;  // env?
	uint8_t* ip;
	size_t stack_index;
};

class VirtualMachine {
private:
	interpret_result run();
	std::vector<Value> stack;
	std::vector<CallFrame> frames;
	std::forward_list<Data> memory;
	// TODO: benchmark map performance here too
	std::list<std::shared_ptr<RuntimeUpvalue>> open_upvalues;
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
	bool call(std::shared_ptr<Closure> function, size_t number_arguments);
	bool call(std::shared_ptr<BuiltinFunction> function, size_t number_arguments);
	std::shared_ptr<RuntimeUpvalue> captureUpvalue(size_t index);
public:
	interpret_result interpret(std::string source);
	Value allocate(std::string string);
	Value allocate(std::shared_ptr<Function> function);
	Value allocate(std::shared_ptr<Closure> closure);
	Value allocate(std::shared_ptr<BuiltinFunction> builtin);
	Value allocate(Pair pair);
	size_t global(std::string key);
	VirtualMachine();
};

#endif