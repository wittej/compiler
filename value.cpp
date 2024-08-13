#include "value.h"
#include "function.h"


/**
 * String representation of Value - used by "print" part of REPL.
 * 
 * @return: string representation of Value to print.
 */
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

/**
 * Convenience function to check the data type of a value.
 * 
 * @param match: data_type to check for.
 * @return: is the Value of data_type 'match'?
 */
bool
Value::match_data_type(data_type match)
{
	return type == value_type::DATA && as.data->type == match;
}

size_t
Data::size()
{
	switch (type) {
		case data_type::PAIR:
			return sizeof(Pair) + sizeof(Data);
		case data_type::FUNCTION:
			using func_pt = std::shared_ptr<Function>;
			return cast<func_pt>()->size() + sizeof(func_pt) + sizeof(Data);
		case data_type::CLOSURE:
			using clos_pt = std::shared_ptr<Closure>;
			return cast<clos_pt>()->size() + sizeof(clos_pt) + sizeof(Data);
		case data_type::STRING:
			return cast<std::string&>().size() + sizeof(Data);
		case data_type::UPVALUE:
			return sizeof(RuntimeUpvalue) + sizeof(Data);
		case data_type::BUILTIN:  // TODO: consolidate into one type
		default:
			return sizeof(Data);
	}
}
