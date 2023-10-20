#include "bytecode.h"

/**
* @param constant : store this Value in the constant table
* @param overflow : output param, index byte 2 - will be clobbered
* @return : index (in constant table) byte 1
*/
uint8_t Chunk::add_constant(Value constant, uint8_t& overflow) {
	constants.push_back(constant);
	uint16_t c = constants.size() - 1;
	overflow = (c > 225) ? static_cast<uint8_t>(c >> 8) : 0;
	return (c > 255) ? static_cast<uint8_t>(c & 255) : static_cast<uint8_t>(c);
};

void Chunk::write(uint8_t op, bool newline) {
	code.push_back(op);
	newlines.push_back(newline);
};