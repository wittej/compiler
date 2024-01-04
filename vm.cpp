
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

// TODO: More complicated behavior
// Will want two functions: one to access global on index, one to access on string
// This one will likely be used by the compiler only - want to initialize to special
// "undefined" Value and add the index of that value to the map. This can then be
// used to quickly update values at runtime.
inline int
VirtualMachine::insert_global(std::string key, Value value)
{
	globals.insert(std::pair<std::string, Value>(key, value));
	return 0;  // TODO: report error as -1
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
		case opcode::DEFINE_GLOBAL: {
			// TODO: instead get the index associated with this global - define that
			Value val = stack_pop();
			data_cast<std::string> str = val.cast_string();
			if (str.error || insert_global(str.value, stack_pop()) != 0)  // TODO: settle on single error-handling paradigm
				return interpret_result::RUNTIME_ERROR;
			stack.push_back(Value(true));  // TEMP
			break;
		}
		case opcode::GET_GLOBAL:
			// TODO: instead get the global at this index
			// Need "undefined" value - similar to how Python does it
			{
				std::string var = std::any_cast<std::string>(stack_pop().as.data->data);  // TODO: method with safety features
				if (!globals.contains(var)) {
					runtime_error("Undefined variable " + var, line);
					return interpret_result::RUNTIME_ERROR;
				}
				stack.push_back(globals[var]);
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