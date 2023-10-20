#include "common.h"
#include "bytecode.h"
#include "debug.h"


int
main(void)
{
	Chunk bytecode = Chunk(100);

	for (int i = 0; i < 256; i++) bytecode.add_constant(i);

	ConstantIndex constant = bytecode.add_constant(1.2);
	if (constant.overflow == 0) {
		bytecode.write(opcode::CONSTANT, true);
		bytecode.write(constant.index, false);
	}
	// TODO: choose endianness lol
	else {
		bytecode.write(opcode::CONSTANT_LONG, true);
		bytecode.write(constant.index, false);
		bytecode.write(constant.overflow, false);
	}

	bytecode.write(opcode::RETURN, true);
	
	disassembleBytecode(bytecode, "Test");
	return 0;
}
