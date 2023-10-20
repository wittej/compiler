#include "debug.h"
#include "value.h"

void
disassembleBytecode(Chunk& bytecode, std::string name)
{
	std::cout << "== " << name << " ==\n";
	size_t line = line = bytecode.line;
	for (size_t offset = 0; offset < bytecode.code.size();) {
		offset = disassembleInstruction(bytecode, offset, line);
	}
}

static size_t
simpleInstruction(std::string name, size_t offset)
{
	std::cout << name << '\n';
	return offset + 1;
}

static size_t
constantInstruction(std::string name, Chunk& bytecode, size_t offset)
{
	uint8_t constant = bytecode.code.at(offset + 1);
	std::cout << name << ' ' << bytecode.constants.at(constant) << '\n';
	return offset + 2;
}

size_t
disassembleInstruction(Chunk& bytecode, size_t offset, size_t& line)
{
	std::cout << std::setfill('0') << std::setw(4) << offset << ' ';
	if (bytecode.newlines.at(offset)) {
		std::cout << std::setfill(' ') << std::setw(4) << line++ << ' ';
	}
	else {
		std::cout << "   | ";
	}
	uint8_t instruction = bytecode.code.at(offset);
	switch (instruction) {
	case opcode::RETURN:
		return simpleInstruction("RETURN", offset);
	case opcode::CONSTANT:
		return constantInstruction("CONSTANT", bytecode, offset);
	default:
		std::cout << "Unknown opcode " << instruction << "\n";
		return offset + 1;
	}
}
