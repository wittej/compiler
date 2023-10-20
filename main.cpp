#include "common.h"
#include "bytecode.h"
#include "debug.h"


int
main(void)
{
	Chunk bytecode = Chunk(100);
	uint8_t constant = bytecode.add_constant(1.2);
	bytecode.write(opcode::CONSTANT, true);
	bytecode.write(constant, false);
	bytecode.write(opcode::RETURN, true);
	disassembleBytecode(bytecode, "Test");
	return 0;
}
