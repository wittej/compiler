#include "common.h"
#include "vm.h"

VirtualMachine VM;

interpret_result
VirtualMachine::interpret(Chunk& bytecode)
{
	ip = bytecode.instructions.data();
}
