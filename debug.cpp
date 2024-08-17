#include "debug.h"
#include "value.h"
#include "function.h"

/**
 * Print a chunk of bytecode in human-readable format.
 * 
 * @param bytecode: bytecode to print.
 * @param name: name of function (if applicable).
 */
void
disassembleBytecode(Chunk& bytecode, std::string name)
{
	size_t line = bytecode.base_line;
	std::cerr << "== " << name << " ==\n";
	for (size_t offset = 0; offset < bytecode.instructions.size();) {
		offset = disassembleInstruction(bytecode, offset, line);
	}
}

/**
 * Print a simple (single-line) instruction.
 * 
 * @param name: human-readable name of instruction.
 * @param offset: location of instruction in bytecode.
 * @return: location of next bytecode instruction.
 */
static size_t
simpleInstruction(std::string name, size_t offset)
{
	std::cerr << name << '\n';
	return offset + 1;
}

// DEPRACATED>
/**
 * Print a constant (including its value) in human-readable format.
 *
 * @param name: human-readable name of instruction.
 * @param bytecode: bytecode where constant is stored.
 * @param offset: location of instruction in bytecode.
 * @return: location of next bytecode instruction.
 */
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

/**
 * Print an instruction with a uint16 component in human-readable format.
 *
 * @param name: human-readable name of instruction.
 * @param bytecode: bytecode where constant is stored.
 * @param offset: location of instruction in bytecode.
 * @return: location of next bytecode instruction.
 */
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


static size_t
closure(std::string name, Chunk& bytecode, size_t offset)
{
	// NB: Endianness is important here
	uint8_t uint8 = bytecode.instructions[offset + 1];
	uint8_t overflow = bytecode.instructions[offset + 2];
	uint16_t uint16 = static_cast<uint16_t>(overflow) * 256 + uint8;

	std::cerr << name << ' ' << uint16 << '\n';
	offset += 3;

	Value data = bytecode.constants[uint16];
	if (!data.match_data_type(data_type::FUNCTION)) {
		std::cerr << "ERROR READING FUNCTION VARIABLES" << '\n';
		return offset;
	}

	std::shared_ptr<Function> func = std::any_cast<std::shared_ptr<Function>>(data.as.data->data);
	for (size_t i = 0; i < func->upvalues; i++) {
		int local = bytecode.instructions[offset++];
		uint8_t index_uint8 = bytecode.instructions[offset++];
		uint8_t index_overflow = bytecode.instructions[offset++];
		uint16_t index = static_cast<uint16_t>(index_overflow) * 256 + index_uint8;
		std::cerr << "   | " << (local ? "local " : "upvalue ") << index << '\n';
	}

	return offset;
}

/**
 * Print an instruction in human-readable format.
 *
 * @param name: human-readable name of instruction.
 * @param bytecode: bytecode where instruction is located.
 * @param offset: location of instruction in bytecode.
 * @return: location of next bytecode instruction.
 */
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
		return longConstantInstruction("CONSTANT", bytecode, offset);
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
	case opcode::SET_GLOBAL:
		return uintInstruction("SET GLOBAL", bytecode, offset);
	case opcode::GET_GLOBAL:
		return uintInstruction("GET GLOBAL", bytecode, offset);
	case opcode::GET_UPVALUE:
		return uintInstruction("GET UPVALUE", bytecode, offset);
	case opcode::SET_UPVALUE:
		return uintInstruction("SET UPVALUE", bytecode, offset);
	case opcode::GET_LOCAL:
		return uintInstruction("GET LOCAL", bytecode, offset);
	case opcode::SET_LOCAL:
		return uintInstruction("SET LOCAL", bytecode, offset);
	case opcode::JUMP:
		return uintInstruction("JUMP", bytecode, offset);
	case opcode::JUMP_IF_FALSE:
		return uintInstruction("JUMP IF FALSE", bytecode, offset);
	case opcode::CALL:
		return uintInstruction("CALL", bytecode, offset);
	case opcode::TAIL_CALL:
		return uintInstruction("TAIL CALL", bytecode, offset);
	case opcode::CLOSURE:
		return closure("CLOSURE", bytecode, offset);
	default:
		int undefined_opcode = static_cast<int>(instruction);
		std::cerr << "Unknown opcode " << undefined_opcode << "\n";
		return offset + 1;
	}
}
