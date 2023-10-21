#include "common.h"
#include "bytecode.h"
#include "debug.h"
#include "vm.h"


int
main(void)
{
	Chunk bytecode = Chunk(100);

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
	
	VirtualMachine().interpret(bytecode);
	disassembleBytecode(bytecode, "Test");
	return 0;
}
