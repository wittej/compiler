#ifndef LISP_FUNCTION_H
#define LISP_FUNCTION_H

#include "common.h"
#include "value.h"

class VirtualMachine;


struct Closure {
	std::shared_ptr<Function> function;
	Closure(std::shared_ptr<Function> function) : function{ function } {};
};


struct Function {
	size_t arity = 0;
	Chunk bytecode;
	std::string name;
	bool anonymous() { return name.size() == 0; }
	Function() : bytecode{ Chunk(0) } {}  // TEMP
};

// TODO: give this some way of throwing an error
struct BuiltinFunction {
	virtual Value call(std::vector<Value>::iterator args, size_t count) = 0;
};

struct BuiltinCons : BuiltinFunction {
private:
	VirtualMachine& vm;
public:
	Value call(std::vector<Value>::iterator args, size_t count);
	BuiltinCons(VirtualMachine& vm) : vm{ vm } {};
};

struct BuiltinAdd : BuiltinFunction {
private:
	VirtualMachine& vm;
public:
	Value call(std::vector<Value>::iterator args, size_t count);
	BuiltinAdd(VirtualMachine& vm) : vm{ vm } {};
};

struct BuiltinEqual : BuiltinFunction {
private:
	VirtualMachine& vm;
public:
	Value call(std::vector<Value>::iterator args, size_t count);
	BuiltinEqual(VirtualMachine& vm) : vm{ vm } {};
};

#endif