#include "bytecode.h"

uint8_t add_constant(Chunk& chunk, Value constant) {
	// TODO: plan for number of constants too large to appear in opcode
	chunk.constants.push_back(constant);
	return chunk.constants.size() - 1;  // TODO: this can overflow - fix
};