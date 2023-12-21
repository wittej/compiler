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

Value
VirtualMachine::stack_peek(size_t depth)
{
	return stack[stack.size() - depth - 1];
}

void
VirtualMachine::runtime_error(std::string message, size_t line)
{
	std::cerr << message << " at line " << line << '\n';
	stack.clear();
}

Value
VirtualMachine::allocate(std::string string)
{
	memory.push_front(Data(string));
	return Value(&memory.front());
}

interpret_result
VirtualMachine::interpret(std::string source)
{
	Chunk bytecode(1);

	// Probably want the compiler to return the bytecode or some sort of struct
	Compiler compiler(source, *this, bytecode);
	if (!compiler.compile()) return interpret_result::COMPILE_ERROR;

	Chunk compiled_bytecode = compiler.get_bytecode();
	return run(compiled_bytecode);
}

// Scheme rules, everything but "false" is truthy
bool
VirtualMachine::truthValue(Value val)
{
	return (val.type == value_type::BOOL) ? val.as.boolean : true;
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
		case opcode::DEFINE_GLOBAL:
			{
				stack.back().print();
				Value var0 = stack_pop();
				// TODO: this is assuming this is a valid string - need to check
				std::string var = std::any_cast<std::string>(var0.as.data->data);
				// TODO: check for defintion
				Value var1 = stack_pop();
				globals.insert(std::pair<std::string, Value>(var, var1));
				stack.push_back(Value(true));  // TEMP
			}
			break;
		case opcode::ADD:
			{
				if (stack_peek(0).type != value_type::NUMBER ||
					stack_peek(1).type != value_type::NUMBER) {
					runtime_error("+: expected numeric operand", line);
					return interpret_result::RUNTIME_ERROR;
				}
				Value b = stack_pop();
				Value a = stack_pop();
				stack.push_back(Value(a.as.number + b.as.number));
			}
			break;
		case opcode::EQUAL:
			{
				if (stack_peek(0).type != value_type::NUMBER ||
					stack_peek(1).type != value_type::NUMBER) {
					runtime_error("=: expected numeric operand", line);
					return interpret_result::RUNTIME_ERROR;
				}
				Value b = stack_pop();
				Value a = stack_pop();
				stack.push_back(Value(a.as.number == b.as.number));
			}
			break;
		case opcode::CONS:
			{
				Value b = stack_pop();
				Value a = stack_pop();
				memory.push_front(Data(Pair(a, b)));
				stack.push_back(Value(&memory.front()));
			}
			break;
		case opcode::NOT:
			stack.push_back(Value(!truthValue(stack_pop())));
			break;
		case opcode::TRUE:
			stack.push_back(Value(true));
			break;
		case opcode::FALSE:
			stack.push_back(Value(false));
			break;
		case opcode::NIL:
			stack.push_back(Value());
			break;
		case opcode::RETURN:
			std::cout << stack_pop().print() << '\n';
			return interpret_result::OK;
		default:
			return interpret_result::RUNTIME_ERROR;
		}
	}
}