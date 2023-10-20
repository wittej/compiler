#include "common.h"
#include "bytecode.h"
#include "debug.h"


int
main(void)
{
	Chunk bytecode = Chunk(100);
	uint8_t overflow = 0;
	uint8_t constant = bytecode.add_constant(1.2, overflow);
	bytecode.write(opcode::CONSTANT, true);
	bytecode.write(constant, false);
	// TODO: choose endianness lol
	if (overflow > 0) bytecode.write(overflow, false);
	bytecode.write(opcode::RETURN, true);
	disassembleBytecode(bytecode, "Test");
	return 0;
}
