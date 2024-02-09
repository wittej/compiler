#include "value.h"
#include "function.h"


std::string
Value::print()
{
	{
		switch (type) {
		case value_type::NUMBER:
			return std::to_string(as.number);
		case value_type::BOOL:
			return std::to_string(as.boolean);
		case value_type::NIL:
			return "nil";
		case value_type::DATA:
			// Temporary - want to be able to print circular data structures, lists in lisp format, etc.
			// May also consider data print - this helps handle printing cons structures
			switch (as.data->type) {
			case data_type::PAIR:
			{
				Pair pair = std::any_cast<Pair>(as.data->data);
				return "(" + pair.car.print() + " . " + pair.cdr.print() + ")";
			}
			case data_type::FUNCTION:
			{
				auto function = std::any_cast<std::shared_ptr<Function>>(as.data->data);
				if (function->anonymous()) {
					return "Function at " + std::to_string(function->bytecode.base_line);
				}
				else {
					return function->name;
				}
			}
			case data_type::CLOSURE:
			{
				auto closure = std::any_cast<std::shared_ptr<Closure>>(as.data->data);
				auto function = std::any_cast<std::shared_ptr<Function>>(closure->function->data);
				if (function->anonymous()) {
					// TODO remove this
					return "Function (CLOSURE) at " + std::to_string(function->bytecode.base_line);
				}
				else {
					return function->name;
				}
			}
			case data_type::BUILTIN:
			{
				auto builtin = std::any_cast<std::shared_ptr<BuiltinFunction>>(as.data->data);
				return "Built-in function " + builtin->name();
			}
			default:
				return "unknown data type";
			};
		default:
			return "unknown type";
		}
	}
}

bool
Value::match_data_type(data_type match)
{
	return type == value_type::DATA && as.data->type == match;
}
