
#include "vm.h"
#include "compiler.h"


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

size_t
VirtualMachine::global(std::string key)
{
	if (global_indexes.contains(key)) return global_indexes[key];
	
	size_t size = globals.size();
	globals.push_back(Value(value_type::UNINITIALIZED));
	global_indexes.insert(std::pair<std::string, size_t>(key, size));
	return size;
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

// TODO: clean up switch block formatting or break into functions

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
		case opcode::DEFINE_GLOBAL: {
				size_t index = stack_pop().as.uint;
				globals[index] = stack_pop();  // TODO: consider unsigned Value
				stack.push_back(Value(true));  // TEMP - assuming this will return
			}
			break;
		case opcode::GET_GLOBAL: {
			// TODO: instead get the global at this index
			// Need "undefined" value - similar to how Python does it
			size_t index = stack_pop().as.uint;
			if (globals[index].type == value_type::UNINITIALIZED) {
				runtime_error("Unintialized variable ", line);
				return interpret_result::RUNTIME_ERROR;
			}
			stack.push_back(globals[index]);
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
			stack.push_back(Value(value_type::NIL));
			break;
		case opcode::POP:
			stack_pop();
			break;
		case opcode::RETURN:
			std::cout << stack_pop().print() << '\n';
			return interpret_result::OK;
		default:
			return interpret_result::RUNTIME_ERROR;
		}
	}
}