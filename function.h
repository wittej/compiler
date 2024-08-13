#ifndef LISP_FUNCTION_H
#define LISP_FUNCTION_H

#include "common.h"
#include "value.h"

class VirtualMachine;

// TODO: during GC implementation, consider whether these pointers to memory
//       objects can be converted to references.

/**
 * Runtime closure representation - contains function and enclosed variables.
 * 
 * @param function: pointer to the function in runtime memory.
 */
struct Closure
{
	Data* function;
	std::shared_ptr<Function> function_ptr();
	std::vector<std::shared_ptr<RuntimeUpvalue>> upvalues;
	size_t size();
	Closure(Data* function) : function{ function } {};
};

/**
 * Runtime function representation - stores information needed for calling the
 * function, and representing closures that enclose this function.
 */
struct Function
{
	size_t arity = 0;
	size_t upvalues = 0;
	Chunk bytecode;
	std::string name;
	bool anonymous() { return name.size() == 0; }
	size_t size();
	Function() : bytecode{ Chunk(0) } {}  // TEMP
};

// TODO: give this some way of throwing an error
// TODO: consider removing VM reference for builtins - shouldn't be needed.

/**
 * Interface for built-in functions (e.g., addition). These are implemented in
 * C++ and can be called during runtime.
 */
struct BuiltinFunction
{
	virtual Value call(std::vector<Value>::iterator args, size_t count) = 0;
	virtual const std::string& name() = 0;
};

/**
 * Supports Pair data structure creation. Requires reference to VM in order to
 * create a memory object.
 */
struct BuiltinCons : BuiltinFunction {
private:
	VirtualMachine& vm;
public:
	Value call(std::vector<Value>::iterator args, size_t count);
	const std::string& name() { return "cons"; };
	BuiltinCons(VirtualMachine& vm) : vm{ vm } {};
};

/**
 * Supports addition.
 */
struct BuiltinAdd : BuiltinFunction {
	Value call(std::vector<Value>::iterator args, size_t count);
	const std::string& name() { return "+"; };
};

/**
 * Supports test for equality.
 */
struct BuiltinEqual : BuiltinFunction {
	Value call(std::vector<Value>::iterator args, size_t count);
	const std::string& name() { return "="; };
};

#endif