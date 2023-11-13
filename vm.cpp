#include "common.h"
#include "vm.h"
#include "compiler.h"
#include "debug.h"

#define DEBUG_TRACE_EXECUTION

Value
VirtualMachine::stack_pop()
{
	Value value = this->stack.back();
	this->stack.pop_back();
	return value;
}

interpret_result
VirtualMachine::interpret(std::string source)
{
	compile(source);
	return interpret_result::OK;
}

interpret_result
VirtualMachine::run(Chunk& bytecode)
{
#ifdef DEBUG_TRACE_EXECUTION
	size_t line = bytecode.line;
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
				double b = stack_pop();
				double a = stack_pop();
				stack.push_back(a + b);
			}
			break;
		case opcode::RETURN:
			std::cout << stack.back() << '\n';
			stack.pop_back();
			return interpret_result::OK;
		default:
			return interpret_result::RUNTIME_ERROR;
		}
	}
}