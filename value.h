#ifndef LISP_VALUE_H
#define LISP_VALUE_H

#include "common.h"
#include "bytecode.h"

enum class value_type {
	BOOL, NIL, NUMBER, DATA, UNINITIALIZED, UNDEFINED
};

enum class data_type {
	PAIR, FUNCTION, BUILTIN, CLOSURE, STRING, UPVALUE
};

struct Data;
struct Function;
struct Closure;
struct BuiltinFunction;
struct RuntimeUpvalue;

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
	// TODO: consider error type per type systems and programming languages
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

struct Data {
	data_type type;
	std::any data;
	Data(Pair pair) : type{ data_type::PAIR }, data{ pair } {}
	Data(std::string string) : type{ data_type::STRING }, data{ string } {}
	// TODO: revisit this
	Data(std::shared_ptr<Function> function) : type{ data_type::FUNCTION }, data{ function } {}
	Data(std::shared_ptr<Closure> closure) : type{ data_type::CLOSURE }, data{ closure } {}
	Data(std::shared_ptr<BuiltinFunction> builtin) : type{ data_type::BUILTIN }, data{ builtin } {}
	Data(std::shared_ptr<RuntimeUpvalue> upvalue) : type{ data_type::UPVALUE }, data{ upvalue } {}
};

// NOTE: pointer at vector doesn't work - just always close off or use an index.
// Also note - GC will need to care about this - maybe.
struct RuntimeUpvalue {
	size_t index;
	Value data = Value(value_type::UNINITIALIZED);
	RuntimeUpvalue(size_t index) : index{ index } {}
};

#endif