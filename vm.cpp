#include "common.h"
#include "vm.h"
#include "debug.h"

#define DEBUG_TRACE_EXECUTION

interpret_result
VirtualMachine::interpret(Chunk& bytecode)
{
	return run(bytecode);
}

interpret_result
VirtualMachine::run(Chunk& bytecode)
{
	uint8_t *ip = bytecode.instructions.data();  // Here for speed for now.
	for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
		disassembleInstruction(bytecode, ip - bytecode.instructions.data());
#endif
		switch (uint8_t instruction = *ip++) {
		case opcode::CONSTANT:
			std::cout << bytecode.constants[*ip++] << '\n';
			break;
		case opcode::RETURN:
			return interpret_result::OK;
		default:
			return interpret_result::RUNTIME_ERROR;
		}
	}
}