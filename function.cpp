#include "function.h"
#include "vm.h"


Value
BuiltinCons::call(std::vector<Value>::iterator args, size_t count)
{
	if (count != 2) return Value(value_type::NIL);  // TEMP
	Value left = *args++;
	Value right = *args++;
	return vm.allocate(Pair(left, right));
}


Value
BuiltinAdd::call(std::vector<Value>::iterator args, size_t count)
{
	if (count != 2) return Value(value_type::NIL);  // TEMP
	Value left = *args++;
	Value right = *args++;

	if (left.type != value_type::NUMBER) return Value(value_type::NIL);  // TEMP
	if (right.type != value_type::NUMBER) return Value(value_type::NIL);  // TEMP

	return Value(left.as.number + right.as.number);
}

Value
BuiltinEqual::call(std::vector<Value>::iterator args, size_t count)
{
	if (count != 2) return Value(value_type::NIL);  // TEMP
	Value left = *args++;
	Value right = *args++;

	if (left.type != value_type::NUMBER) return Value(value_type::NIL);  // TEMP
	if (right.type != value_type::NUMBER) return Value(value_type::NIL);  // TEMP

	return Value(left.as.number == right.as.number);
}
