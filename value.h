#ifndef LISP_VALUE_H
#define LISP_VALUE_H

#include "common.h"

enum class ValueType {
	BOOL,
	NIL,
	NUMBER
};

struct Value {
	ValueType type;
	union {
		bool boolean;
		double number;
	} as;
	Value(bool val) : type{ ValueType::BOOL }, as{ .boolean=val } {}
	Value(double val) : type{ ValueType::NUMBER }, as{ .number=val } {}
	Value() : type{ ValueType::NIL }, as{ .boolean=false } {}
};

#endif