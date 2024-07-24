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
				switch (as.data->type) {
						// TODO: data structures with cycles
						case data_type::PAIR: {
							auto pair = std::any_cast<Pair>(as.data->data);
							auto car = pair.car.print();
							auto cdr = pair.cdr.print();
							return "(" + car + " . " + cdr + ")";
						}

						case data_type::FUNCTION: {
							using func_t = std::shared_ptr<Function>;
							auto function = as.data->cast<func_t>();

							if (function->anonymous()) {
								auto line = function->bytecode.base_line;
								return "Function at " + std::to_string(line);
							}

							else {
								return function->name;
							}
						}
			
						case data_type::CLOSURE: {
							using clos_t = std::shared_ptr<Closure>;
							using func_t = std::shared_ptr<Function>;
							auto closure = as.data->cast<clos_t>();
							auto function = closure->function->cast<func_t>();

							if (function->anonymous()) {
								auto line = function->bytecode.base_line;
								return "Function at " + std::to_string(line);
							}

							else {
								return function->name;
							}
						}
				
						case data_type::BUILTIN: {
							using bfunc_t = std::shared_ptr<BuiltinFunction>;
							auto builtin = as.data->cast<bfunc_t>();

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
