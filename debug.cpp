#include "debug.h"
#include "value.h"

void
disassembleBytecode(Chunk& bytecode, std::string name)
{
	size_t line = bytecode.base_line;
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
	uint8_t index = bytecode.instructions[offset + 1];

	Value value = bytecode.constants[index];
	switch (value.type) {
	case value_type::NUMBER:
		std::cerr << name << ' ' << value.as.number << '\n';
		break;
	case value_type::BOOL:
		std::cerr << name << ' ' << value.as.boolean << '\n';
		break;
	case value_type::NIL:
		std::cerr << name << ' ' << "nil" << '\n';
		break;
	case value_type::DATA:
		std::cerr << name << ' ' << "data" << '\n';
		break;
	default:
		std::cerr << name << " unknown type" << '\n';
	}

	return offset + 2;
}

static size_t
longConstantInstruction(std::string name, Chunk& bytecode, size_t offset)
{
	// NB: Endianness is important here
	uint8_t constant = bytecode.instructions[offset + 1];
	uint8_t overflow = bytecode.instructions[offset + 2];
	uint16_t index = static_cast<uint16_t>(overflow) * 256 + constant;
	
	Value value = bytecode.constants[index];
	switch (value.type) {
	case value_type::NUMBER:
		std::cerr << name << ' ' << value.as.number << '\n';
		break;
	case value_type::BOOL:
		std::cerr << name << ' ' << value.as.boolean << '\n';
		break;
	case value_type::NIL:
		std::cerr << name << ' ' << "nil" << '\n';
		break;
	case value_type::DATA:
		std::cerr << name << ' ' << "data" << '\n';
		break;
	default:
		std::cerr << name << " unknown type" << '\n';
	}

	return offset + 3;
}

static size_t
uintInstruction(std::string name, Chunk& bytecode, size_t offset)
{
	// NB: Endianness is important here
	uint8_t uint8 = bytecode.instructions[offset + 1];
	uint8_t overflow = bytecode.instructions[offset + 2];
	uint16_t uint16 = static_cast<uint16_t>(overflow) * 256 + uint8;

	std::cerr << name << ' ' << uint16 << '\n';

	return offset + 3;
}


size_t
disassembleInstruction(Chunk& bytecode, size_t offset, size_t& line)
{

	if (bytecode.newlines[offset]) {
		std::cerr << std::setfill(' ') << std::setw(4) << line++ << ' ';
	}
	else {
		std::cerr << "   | ";
	}

	std::cerr << std::setfill('0') << std::setw(4) << offset << ' ';

	uint8_t instruction = bytecode.instructions[offset];
	switch (instruction) {
	case opcode::RETURN:
		return simpleInstruction("RETURN", offset);
	case opcode::CONSTANT:
		return constantInstruction("CONSTANT", bytecode, offset);
	case opcode::CONSTANT_LONG:
		return longConstantInstruction("CONSTANT_LONG", bytecode, offset);
	case opcode::ADD:
		return simpleInstruction("ADD", offset);
	case opcode::EQUAL:
		return simpleInstruction("EQUAL", offset);
	case opcode::CONS:
		return simpleInstruction("CONS", offset);
	case opcode::TRUE:
		return simpleInstruction("TRUE", offset);
	case opcode::FALSE:
		return simpleInstruction("FALSE", offset);
	case opcode::NIL:
		return simpleInstruction("NIL", offset);
	case opcode::NOT:
		return simpleInstruction("NOT", offset);
	case opcode::POP:
		return simpleInstruction("POP", offset);
	case opcode::DEFINE_GLOBAL:
		return uintInstruction("DEFINE GLOBAL", bytecode, offset);
	case opcode::GET_GLOBAL:
		return uintInstruction("GET GLOBAL", bytecode, offset);
	case opcode::GET_LOCAL:
		return uintInstruction("GET LOCAL", bytecode, offset);
	case opcode::JUMP:
		return uintInstruction("JUMP", bytecode, offset);
	case opcode::JUMP_IF_FALSE:
		return uintInstruction("JUMP IF FALSE", bytecode, offset);
	case opcode::CALL:
		return uintInstruction("CALL", bytecode, offset);
	case opcode::CLOSURE:
		return uintInstruction("CLOSURE", bytecode, offset);
	default:
		int undefined_opcode = static_cast<int>(instruction);
		std::cerr << "Unknown opcode " << undefined_opcode << "\n";
		return offset + 1;
	}
}
