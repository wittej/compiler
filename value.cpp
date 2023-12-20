#include "value.h"

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
			switch (as.data->type) {
			case data_type::PAIR:
			{
				Pair pair = std::any_cast<Pair>(as.data->data);
				return "(" + pair.car.print() + " . " + pair.cdr.print() + ")";
			}
			default:
				return "unknown data type";
			};
		default:
			return "unknown type";
		}
	}
}