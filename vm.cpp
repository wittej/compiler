#include "common.h"
#include "vm.h"
#include "compiler.h"
#include "debug.h"


Value
VirtualMachine::stack_pop()
{
	Value value = stack.back();
	stack.pop_back();
	return value;
}

void
VirtualMachine::runtime_error(std::string message)
{
	std::cerr << message << '\n';
}

interpret_result
VirtualMachine::interpret(std::string source)
{
	Chunk bytecode(1);

	// Probably want the compiler to return the bytecode or some sort of struct
	Compiler compiler(source, bytecode);
	if (!compiler.compile()) return interpret_result::COMPILE_ERROR;

	Chunk compiled_bytecode = compiler.get_bytecode();
	return run(compiled_bytecode);
}

interpret_result
VirtualMachine::run(Chunk& bytecode)
{
#ifdef DEBUG_TRACE_EXECUTION
	size_t line = bytecode.base_line;
#endif
	uint8_t *ip = bytecode.instructions.data();  // Here for speed for now.

	for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
		disassembleInstruction(bytecode, ip - bytecode.instructions.data(), line);
		if (bytecode.newlines[ip - bytecode.instructions.data()]) ++line;
#endif
		switch (uint8_t instruction = *ip++) {
		case opcode::CONSTANT:
			stack.push_back(bytecode.constants[*ip++]);
			break;
		case opcode::CONSTANT_LONG:
			{
				uint8_t constant = *ip++;
				uint8_t overflow = *ip++;
				uint16_t index = static_cast<uint16_t>(overflow) * 256 + constant;
				stack.push_back(bytecode.constants[index]);
			}
			break;
		case opcode::ADD:
			{
				Value b = stack_pop();
				Value a = stack_pop();
				if (a.type != ValueType::NUMBER || b.type != ValueType::NUMBER) {
					runtime_error("+: expected numeric operand");
					return interpret_result::RUNTIME_ERROR;
				}
				stack.push_back(a.as.number + b.as.number);
			}
			break;
		case opcode::RETURN:
			{
				Value value = stack_pop();
				switch (value.type) {
				case ValueType::NUMBER:
					std::cout << value.as.number << '\n';
					break;
				case ValueType::BOOL:
					std::cout << value.as.boolean << '\n';
					break;
				case ValueType::NIL:
					std::cout << "nil" << '\n';
					break;
				default:
					std::cout << "unknown type" << '\n';
					return interpret_result::RUNTIME_ERROR;
				}
			}
			return interpret_result::OK;
		default:
			return interpret_result::RUNTIME_ERROR;
		}
	}
}