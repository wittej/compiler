#include "debug.h"

void
disassembleBytecode(std::vector<opcode>& bytecode, std::string name)
{
	std::cout << "== " << name << " ==\n";
	for (size_t offset = 0; offset < bytecode.size();) {
		offset = disassembleInstruction(bytecode, offset);
	}
}

static size_t
simpleInstruction(std::string name, size_t offset)
{
	std::cout << name << "\n";
	return offset + 1;
}

size_t
disassembleInstruction(std::vector<opcode>& bytecode, size_t offset)
{
	std::cout << std::setfill('0') << std::setw(4) << offset;
	opcode instruction = bytecode.at(offset);
	switch (instruction) {
	case opcode::RETURN:
		return simpleInstruction("RETURN", offset);
	default:
		std::cout << "Unknown opcode " << static_cast<uint8_t>(instruction) << "\n";
		return offset + 1;
	}
}
