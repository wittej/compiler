#include "bytecode.h"

uint8_t Chunk::add_constant(Value constant) {
	// TODO: plan for number of constants too large to appear in opcode
	constants.push_back(constant);
	return constants.size() - 1;  // TODO: this can overflow - fix
};

void Chunk::write(uint8_t op, size_t line) {
	code.push_back(op);
	lines.push_back(line);
};