#ifndef LISP_VALUE_H
#define LISP_VALUE_H

#include "common.h"
#include "memory.h"

enum class value_type {
	BOOL,
	NIL,
	NUMBER,
	PAIR
};

struct Pair;

struct Value {
	value_type type;
	union {
		bool boolean;
		double number;
		Pair* pair;
	} as;
	Value(bool val) : type{ value_type::BOOL }, as{ .boolean=val } {}
	Value(double val) : type{ value_type::NUMBER }, as{ .number=val } {}
	Value(Pair* val) : type{ value_type::PAIR }, as{ .pair = val } {}
	Value() : type{ value_type::NIL }, as{ .boolean=false } {}
	std::string print();
};

struct Pair {
	Value car;
	Value cdr;
	Pair(Value car, Value cdr) : car{ car }, cdr{ cdr } {};
};

#endif