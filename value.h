#ifndef LISP_VALUE_H
#define LISP_VALUE_H

#include "common.h"

enum class value_type {
	BOOL, NIL, NUMBER, UINT, DATA, UNINITIALIZED, UNDEFINED
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
		size_t uint;
		Data* data;
		value_type type;
	} as;
	Value(bool val) : type{ value_type::BOOL }, as{ .boolean=val } {}
	Value(double val) : type{ value_type::NUMBER }, as{ .number=val } {}
	Value(size_t val) : type{ value_type::UINT }, as{ .uint=val } {}
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