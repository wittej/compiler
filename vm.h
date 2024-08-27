#ifndef LISP_VM_H
#define LISP_VM_H

#include "common.h"
#include "bytecode.h"
#include "value.h"
#include "debug.h"
#include "function.h"

constexpr size_t RECURSION_MAX = 64;
constexpr size_t STACK_MAX = 256 * RECURSION_MAX;

// TODO: correct return process for blank line

enum class interpret_result {
	OK,
	COMPILE_ERROR,
	RUNTIME_ERROR,
};

// TODO: index instead of pointer
/**
 * Call with closure being executed, instruction being executed, and location
 * of frame on stack (starting at index of the closure).
 */
struct CallFrame {
	std::shared_ptr<Closure> closure;  // env?
	uint8_t* ip;
	size_t stack_index;
};

/**
 * Manages memory for runtime - handles allocation nad garbage collection.
 */
class Memory {
	VirtualMachine& vm;
	std::forward_list<Data> memory;
	std::queue<Data*> gc_worklist;
	void advance_worklist();
	size_t collect_garbage();
	size_t gc_threshold = 4;
	size_t gc_size = 0;
public:
	bool gc_active = false;
	Value allocate(Data object);
	void gc_mark(Value val);
	void gc_mark(Data* data);
	void gc_mark(std::shared_ptr<Closure> clos);
	Memory(VirtualMachine& vm) : vm{ vm } {};
};

/**
 * Executes compiled bytecode. Represents state of program execution.
 */
class VirtualMachine {
private:
	interpret_result run();
	std::vector<Value> stack;
	std::vector<CallFrame> frames;
	Memory memory{ *this };

	// TODO: benchmark map performance here too
	std::list<std::shared_ptr<RuntimeUpvalue>> open_upvalues;
	// TODO: benchmark vector vs. map performance here
	// Hybrid solution - string-index map, value vector globals
	std::unordered_map<std::string, size_t> global_indexes;
	std::vector<Value> globals;
	Value stack_pop();
	Value stack_peek(size_t depth);
	void runtime_error(std::string message, size_t line);
	bool truth_value(Value val);
	uint16_t read_uint16_and_update_ip(uint8_t*& ip);  // TODO: move this to util?
	bool call(size_t number_arguments);
	bool call(const std::shared_ptr<Closure> function, size_t number_arguments);
	bool call(std::shared_ptr<BuiltinFunction> function, size_t number_arguments);
	std::shared_ptr<RuntimeUpvalue> capture_upvalue(size_t index);
	void disassemble_current_instruction(size_t line);
	void close_last_frame_upvalues();
	void global_builtin(std::shared_ptr<BuiltinFunction> builtin);
public:
	interpret_result interpret(std::string& source);
	Value allocate(std::string string);
	Value allocate(std::shared_ptr<Function> function);
	Value allocate(std::shared_ptr<Closure> closure);
	Value allocate(std::shared_ptr<BuiltinFunction> builtin);
	Value allocate(Pair pair);
	size_t global(const std::string key);
	bool check_global(const std::string key);
	void gc_mark_roots();
	VirtualMachine();
};

#endif