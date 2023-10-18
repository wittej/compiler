#include "common.h"
#include "bytecode.h"
#include "debug.h"


int
main(void)
{
	std::vector<opcode> bytecode;
	bytecode.push_back(opcode::RETURN);
	disassembleBytecode(bytecode, "Test");
	return 0;
}
