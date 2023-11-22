#include "bytecode.h"

/**
* NB: this one's pretty dangerous. Easy to reason through, but lots of casts
* and bit manipulation.
* 
* @param constant : store this Value in the constant table
* @return : index in constant table
*/
ConstantIndex Chunk::add_constant(Value const constant) {
	constants.push_back(constant);
	uint16_t c = static_cast<uint16_t>(constants.size() - 1);
	return ConstantIndex{
		.index = static_cast<uint8_t>(c & 255),
		.overflow = static_cast<uint8_t>(c >> 8)
	};
};

// NOTE: may want to alter newline-based system due to blank lines
void Chunk::write(uint8_t op, size_t line) {
	instructions.push_back(op);
	if (line > max_line) {
		newlines.push_back(true);
		max_line = line;
	}
	else {
		newlines.push_back(false);
	}
};