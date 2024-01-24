#ifndef LISP_VALUE_H
#define LISP_VALUE_H

#include "common.h"
#include "bytecode.h"

enum class value_type {
	BOOL, NIL, NUMBER, DATA, UNINITIALIZED, UNDEFINED
};

enum class data_type {
	PAIR, FUNCTION, STRING
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
};

struct Pair {
	Value car;
	Value cdr;
	Pair(Value car, Value cdr) : car{ car }, cdr{ cdr } {};
};

// TODO: consider moving function to own header file
// TODO: consider visitor pattern for function / program dispatch

struct Function {
	size_t arity = 0;
	Chunk bytecode;
	std::string name;
	bool anonymous() { return name.size() == 0; }
	Function() : bytecode{ Chunk(0) } {}  // TEMP
};

struct Data {
	data_type type;
	std::any data;
	Data(Pair pair) : type{ data_type::PAIR }, data{ pair } {}
	Data(std::string string) : type{ data_type::STRING }, data{ string } {}
	Data(Function function) : type{ data_type::FUNCTION }, data{ function } {}
};

#endif