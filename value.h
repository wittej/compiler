#ifndef LISP_VALUE_H
#define LISP_VALUE_H

#include "common.h"
#include "bytecode.h"

class VirtualMachine;  // TODO: move this and functions to own file

enum class value_type {
	BOOL, NIL, NUMBER, DATA, UNINITIALIZED, UNDEFINED
};

enum class data_type {
	PAIR, FUNCTION, BUILTIN, STRING
};

struct Data;

struct Value {
	value_type type;
	union {
		bool boolean;
		double number;
		Data* data;
		value_type type;
	} as;
	Value(bool val) : type{ value_type::BOOL }, as{ .boolean=val } {}
	Value(double val) : type{ value_type::NUMBER }, as{ .number=val } {}
	Value(Data* val) : type{ value_type::DATA }, as{ .data=val } {}
	Value(value_type singleton_type) {
		if (singleton_type == value_type::NIL ||
			singleton_type == value_type::UNINITIALIZED) {
			type = singleton_type;
			as = { .type = singleton_type };
		}
		else {
			type = value_type::UNDEFINED;
			as = { .type = value_type::UNDEFINED };
		}
	}
	std::string print();
	bool match_data_type(data_type match);
};

struct Pair {
	Value car;
	Value cdr;
	Pair(Value car, Value cdr) : car{ car }, cdr{ cdr } {};
};

// TODO: consider moving function to own header file
// TODO: consider visitor pattern for function / program dispatch
// TOOD: right abstraction might be a bytecode subclass with arity / name?

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

struct Data {
	data_type type;
	std::any data;
	Data(Pair pair) : type{ data_type::PAIR }, data{ pair } {}
	Data(std::string string) : type{ data_type::STRING }, data{ string } {}
	// TODO: revisit this
	Data(std::shared_ptr<Function> function) : type{ data_type::FUNCTION }, data{ function } {}
	Data(std::shared_ptr<BuiltinFunction> builtin) : type{ data_type::BUILTIN }, data{ builtin } {}
};

#endif