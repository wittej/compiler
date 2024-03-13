#ifndef LISP_FUNCTION_H
#define LISP_FUNCTION_H

#include "common.h"
#include "value.h"

class VirtualMachine;


struct Closure {
	Data* function;  // TODO: reconsider - idea is to make GC easier
	std::shared_ptr<Function> function_ptr();
	std::vector<std::shared_ptr<RuntimeUpvalue>> upvalues;
	Closure(Data* function) : function{ function } {};
};

struct Function {
	size_t arity = 0;
	size_t upvalues = 0;  // Make sure this goes here and not in closure
	Chunk bytecode;
	std::string name;
	bool anonymous() { return name.size() == 0; }
	Function() : bytecode{ Chunk(0) } {}  // TEMP
};

// TODO: give this some way of throwing an error
struct BuiltinFunction {
	virtual Value call(std::vector<Value>::iterator args, size_t count) = 0;
	virtual std::string name() = 0;
};

struct BuiltinCons : BuiltinFunction {
private:
	VirtualMachine& vm;
public:
	Value call(std::vector<Value>::iterator args, size_t count);
	std::string name() { return "cons"; };
	BuiltinCons(VirtualMachine& vm) : vm{ vm } {};
};

struct BuiltinAdd : BuiltinFunction {
private:
	VirtualMachine& vm;
public:
	Value call(std::vector<Value>::iterator args, size_t count);
	std::string name() { return "+"; };
	BuiltinAdd(VirtualMachine& vm) : vm{ vm } {};
};

struct BuiltinEqual : BuiltinFunction {
private:
	VirtualMachine& vm;
public:
	Value call(std::vector<Value>::iterator args, size_t count);
	std::string name() { return "="; };
	BuiltinEqual(VirtualMachine& vm) : vm{ vm } {};
};

#endif