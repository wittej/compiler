#ifndef LISP_VALUE_H
#define LISP_VALUE_H

#include "common.h"

enum class value_type {
	BOOL, NIL, NUMBER, DATA, UNINITIALIZED
};

enum class data_type {
	PAIR, STRING
};

struct Data;

template<typename T>
struct data_cast {
	T value;
	bool error;
};

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
		// Make sure it's a valid singleton type.
		if (singleton_type == value_type::NIL) {
			type = singleton_type;
			as = { .type = singleton_type };
		}
		else {
			type = value_type::UNINITIALIZED;
			as = { .type = value_type::UNINITIALIZED };
		}
	}
	std::string print();
	data_cast<std::string> cast_string();
};

struct Pair {
	Value car;
	Value cdr;
	Pair(Value car, Value cdr) : car{ car }, cdr{ cdr } {};
};

struct Data {
	data_type type;
	std::any data;
	Data(Pair pair) : type{ data_type::PAIR }, data{ pair } {}
	Data(std::string string) : type{ data_type::STRING }, data{ string } {}
};

#endif