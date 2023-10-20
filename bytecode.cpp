#include "bytecode.h"

/**
* NB: this one's pretty dangerous. Easy to reason through, but lots of casts
* and bit manipulation.
* 
* @param constant : store this Value in the constant table
* @return : index in constant table
*/
constant_index Chunk::add_constant(Value const constant) {
	constants.push_back(constant);
	uint16_t c = static_cast<uint16_t>(constants.size() - 1);
	return constant_index{
		.index = static_cast<uint8_t>(c & 255),
		.overflow = static_cast<uint8_t>(c >> 8)
	};
};

void Chunk::write(uint8_t op, bool newline) {
	code.push_back(op);
	newlines.push_back(newline);
};