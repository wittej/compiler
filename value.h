#ifndef LISP_VALUE_H
#define LISP_VALUE_H

#include "common.h"
#include "memory.h"

enum class value_type {
	BOOL,
	NIL,
	NUMBER,
	DATA
};

struct Data;

struct Value {
	value_type type;
	union {
		bool boolean;
		double number;
		Data* data;
	} as;
	Value(bool val) : type{ value_type::BOOL }, as{ .boolean=val } {}
	Value(double val) : type{ value_type::NUMBER }, as{ .number=val } {}
	Value(Data* val) : type{ value_type::DATA }, as{ .data=val } {}
	Value() : type{ value_type::NIL }, as{ .boolean=false } {}
	std::string print();
};

struct Pair {
	Value car;
	Value cdr;
	Pair(Value car, Value cdr) : car{ car }, cdr{ cdr } {};
};

enum class data_type {
	PAIR, STRING
};

struct Data {
	data_type type;
	std::any data;
	Data(Pair pair) : type{ data_type::PAIR }, data{ pair } {}
	Data(std::string string) : type{ data_type::STRING }, data{ string } {}
};

#endif