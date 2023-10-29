#include "debug.h"
#include "value.h"

void
disassembleBytecode(Chunk& bytecode, std::string name)
{
	size_t line = bytecode.line;
	std::cerr << "== " << name << " ==\n";
	for (size_t offset = 0; offset < bytecode.instructions.size();) {
		offset = disassembleInstruction(bytecode, offset, line);
	}
}

static size_t
simpleInstruction(std::string name, size_t offset)
{
	std::cerr << name << '\n';
	return offset + 1;
}

static size_t
constantInstruction(std::string name, Chunk& bytecode, size_t offset)
{
	uint8_t constant = bytecode.instructions.at(offset + 1);
	std::cerr << name << ' ' << bytecode.constants.at(constant) << '\n';
	return offset + 2;
}

static size_t
longConstantInstruction(std::string name, Chunk& bytecode, size_t offset)
{
	// NB: Endianness is important here
	uint8_t constant = bytecode.instructions.at(offset + 1);
	uint8_t overflow = bytecode.instructions.at(offset + 2);
	uint16_t index = static_cast<uint16_t>(overflow) * 256 + constant;
	std::cerr << name << ' ' << bytecode.constants.at(index) << '\n';
	return offset + 3;
}

size_t
disassembleInstruction(Chunk& bytecode, size_t offset, size_t& line)
{

	if (bytecode.newlines.at(offset)) {
		std::cerr << std::setfill(' ') << std::setw(4) << line++ << ' ';
	}
	else {
		std::cerr << "   | ";
	}

	std::cerr << std::setfill('0') << std::setw(4) << offset << ' ';

	uint8_t instruction = bytecode.instructions.at(offset);
	switch (instruction) {
	case opcode::RETURN:
		return simpleInstruction("RETURN", offset);
	case opcode::CONSTANT:
		return constantInstruction("CONSTANT", bytecode, offset);
	case opcode::CONSTANT_LONG:
		return longConstantInstruction("CONSTANT_LONG", bytecode, offset);
	default:
		int undefined_opcode = static_cast<int>(instruction);
		std::cerr << "Unknown opcode " << undefined_opcode << "\n";
		return offset + 1;
	}
}
