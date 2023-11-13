#include "common.h"
#include "bytecode.h"
#include "debug.h"
#include "vm.h"


int
main(void)
{
	Chunk bytecode = Chunk(100);

	for (int i = 0; i < 256; i++) bytecode.add_constant(0.);

	ConstantIndex constant = bytecode.add_constant(1.2);
	if (constant.overflow == 0) {
		bytecode.write(opcode::CONSTANT, true);
		bytecode.write(constant.index, false);
	}
	else {
		bytecode.write(opcode::CONSTANT_LONG, true);
		bytecode.write(constant.index, false);
		bytecode.write(constant.overflow, false);
	}

	constant = bytecode.add_constant(1.3);
	if (constant.overflow == 0) {
		bytecode.write(opcode::CONSTANT, true);
		bytecode.write(constant.index, false);
	}
	else {
		bytecode.write(opcode::CONSTANT_LONG, true);
		bytecode.write(constant.index, false);
		bytecode.write(constant.overflow, false);
	}

	bytecode.write(opcode::ADD, true);

	bytecode.write(opcode::RETURN, true);
	
	interpret_result result = VirtualMachine().interpret(bytecode);
	switch (result) {
	case interpret_result::OK:
		std::cout << "OK\n";
		break;
	case interpret_result::RUNTIME_ERROR:
		std::cout << "RUNTIME ERROR\n";
		break;
	default:
		std::cout << "UNKNOWN RESULT\n";
	}
	disassembleBytecode(bytecode, "Test");
	return 0;
}
