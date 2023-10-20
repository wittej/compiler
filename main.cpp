#include "common.h"
#include "bytecode.h"
#include "debug.h"


int
main(void)
{
	Chunk bytecode;
	uint8_t constant = add_constant(bytecode, 1.2);
	bytecode.code.push_back(opcode::CONSTANT);
	bytecode.code.push_back(constant);
	bytecode.code.push_back(opcode::RETURN);
	disassembleBytecode(bytecode, "Test");
	return 0;
}
