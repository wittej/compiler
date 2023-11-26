#ifndef LISP_VALUE_H
#define LISP_VALUE_H

#include "common.h"

enum class value_type {
	BOOL,
	NIL,
	NUMBER,
	LIST
};

struct Cons;

struct Value {
	value_type type;
	union {
		bool boolean;
		double number;
		Cons* list;
	} as;
	Value(bool val) : type{ value_type::BOOL }, as{ .boolean=val } {}
	Value(double val) : type{ value_type::NUMBER }, as{ .number=val } {}
	Value(Cons* val) : type{ value_type::LIST }, as{ .list = val } {}
	Value() : type{ value_type::NIL }, as{ .boolean=false } {}
};

struct Cons {
	Value car;
	Value cdr;
	Cons(Value car, Value cdr) : car{ car }, cdr{ cdr } {};
};

#endif