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
	void print() {  // TODO: move to own file
		switch (type) {
		case value_type::NUMBER:
			std::cout << as.number << '\n';
			break;
		case value_type::BOOL:
			std::cout << as.boolean << '\n';
			break;
		case value_type::NIL:
			std::cout << "nil" << '\n';
			break;
		case value_type::PAIR:
			// Temporary - want to be able to print circular data structures, lists in lisp format, etc.
			std::cout << "(";
			as.pair->car.print();
			std::cout << " . ";
			as.pair->cdr.print();
			std::cout << ")\n";
			break;
		default:
			std::cout << "unknown type" << '\n';
		}
	}
};

struct Pair {
	Value car;
	Value cdr;
	Pair(Value car, Value cdr) : car{ car }, cdr{ cdr } {};
};

#endif