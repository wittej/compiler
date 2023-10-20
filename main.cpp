#include "common.h"
#include "bytecode.h"
#include "debug.h"


int
main(void)
{
	Chunk bytecode;
	uint8_t constant = bytecode.add_constant(1.2);
	bytecode.write(opcode::CONSTANT, 1);
	bytecode.write(constant, 1);
	bytecode.write(opcode::RETURN, 2);
	disassembleBytecode(bytecode, "Test");
	return 0;
}
