#include "bytecode.h"
#include "value.h"

/**
* Add a Value constant and return its index.
* 
* @param constant : store this Value in the constant table
* @return : index in constant table
*/
uint16_t
Chunk::add_constant(Value const constant)
{
	// TODO: check for UINT16_MAX?
	constants.push_back(constant);
	return static_cast<uint16_t>(constants.size() - 1);
};

// NOTE: may want to alter newline-based system due to blank lines
void
Chunk::write(uint8_t op, size_t line)
{
	instructions.push_back(op);
	if (line > max_line) {
		newlines.push_back(true);
		max_line = line;
	}
	else {
		newlines.push_back(false);
	}
};

void
Chunk::replace(size_t index, uint8_t op)
{
	instructions[index] = op;
}