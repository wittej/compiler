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
