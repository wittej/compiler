#ifndef LISP_VALUE_H
#define LISP_VALUE_H

#include "common.h"

enum class value_type {
	BOOL,
	NIL,
	NUMBER
};

struct Value {
	value_type type;
	union {
		bool boolean;
		double number;
	} as;
	Value(bool val) : type{ value_type::BOOL }, as{ .boolean=val } {}
	Value(double val) : type{ value_type::NUMBER }, as{ .number=val } {}
	Value() : type{ value_type::NIL }, as{ .boolean=false } {}
};

#endif