#include "value.h"

void
Value::print()
{
	{  // TODO: move to own file
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
}