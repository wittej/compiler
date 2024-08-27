#include "function.h"
#include "vm.h"

/**
 * Convenience function for garbage collector.
 *
 * @return: memory footprint of this Data object.
 */
size_t
Function::size()
{
	return sizeof(*this) + name.size() + bytecode.vector_size();
}

/**
 * Convenience function for garbage collector.
 *
 * @return: memory footprint of this Data object.
 */
size_t
Closure::size()
{
	return sizeof(*this);
}

/**
 * Convenience function for retrieving shared pointer to enclosed function.
 * 
 * @return: shared pointer to enclosed function.
 */
std::shared_ptr<Function>
Closure::function_ptr()
{
	return std::any_cast<std::shared_ptr<Function>>(function->data);
}


/**
 * Generates (and allocates in VM memory) a Pair data structure.
 * 
 * @param args: stack location of first argument.
 * @param count: number of arguments - must be 2.
 * @return: Value with a pointer to the newly allocated Pair.
 */
Value
BuiltinCons::call(std::vector<Value>::iterator args, size_t count)
{
	if (count != 2) return Value(value_type::NIL);  // TEMP
	Value left = *args++;
	Value right = *args++;
	return vm.allocate(Pair(left, right));
}

// TODO: support for variable size

/**
 * Adds numeric Values and returns the sum.
 * 
 * @param args: stack location of first argument.
 * @param count: number of arguments.
 * @return: Value - numeric.
 */
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

/**
 * Tests numeric values for equality.
 *
 * @param args: stack location of first argument.
 * @param count: number of arguments.
 * @return: Value with boolean - are the arguments equal?
 */
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
