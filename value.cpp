#include "value.h"
#include "vm.h"  // TODO: move this and functions to their own file

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
			case data_type::FUNCTION:
			{
				std::shared_ptr<Function> function = std::any_cast<std::shared_ptr<Function>>(as.data->data);
				if (function->anonymous()) {
					return "Function at " + std::to_string(function->bytecode.base_line);
				}
				else {
					return function->name;
				}
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


Value
BuiltinCons::call(std::vector<Value>::iterator args, size_t count)
{
	if (count != 2) return Value(value_type::NIL);  // TEMP
	Value left = *args++;
	Value right = *args++;
	return vm.allocate(Pair(left, right));
}
