
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

// TODO: better line handling
void
VirtualMachine::runtime_error(std::string message, size_t line)
{
	std::cerr << message << '\n';

	for (size_t i = frames.size(); i > 0; i--) {
		CallFrame& frame = frames[i - 1];
		size_t instruction = frame.ip - frame.function->bytecode.instructions.data() - 1;
		std::cerr << "in " << frame.function->name << '\n';  // TODO: anonymous function names
	}

	stack.clear();  // TODO - clear func?
}

Value
VirtualMachine::allocate(std::string string)
{
	memory.push_front(Data(string));
	return Value(&memory.front());
}

Value
VirtualMachine::allocate(std::shared_ptr<Function> function)
{
	memory.push_front(Data(function));
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

// NB: will mutate ip
// TODO: think on name - want to be exceptionally clear that ip state changes
// TODO: consider if this is a good idea in general
inline uint16_t
VirtualMachine::read_uint16_and_update_ip(uint8_t*& ip)
{
	uint8_t constant = *ip++;
	uint8_t overflow = *ip++;
	return static_cast<uint16_t>(overflow) * 256 + constant;
}

interpret_result
VirtualMachine::interpret(std::string source)
{
	// Probably want the compiler to return the bytecode or some sort of struct
	Compiler compiler(source, *this);
	std::shared_ptr<Function> function = compiler.get_function();
	if (function == nullptr) return interpret_result::COMPILE_ERROR;

	// TODO: revisit this?
	Data script = Data(function);
	stack.push_back(Value(&script));
	call(0);

	return run();
}

// Scheme rules, everything but "false" is truthy
bool
VirtualMachine::truthValue(Value val)
{
	return (val.type == value_type::BOOL) ? val.as.boolean : true;
}

bool
VirtualMachine::call(size_t number_arguments)
{
	Value val = stack_peek(number_arguments);
	if (!val.match_data_type(data_type::FUNCTION)) return false;

	std::shared_ptr<Function> function = std::any_cast<std::shared_ptr<Function>>(val.as.data->data);
	CallFrame frame{
		.function = function,
		.ip = &function->bytecode.instructions[0],
		.stack_index = stack.size() - number_arguments - 1 };

	frames.push_back(frame);
	return true;
}

// TODO: clean up switch block formatting or break into functions

interpret_result
VirtualMachine::run()
{
#ifdef DEBUG_TRACE_EXECUTION
	size_t line = frames.back().function->bytecode.base_line;
#endif

	for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
		disassembleInstruction(frames.back().function->bytecode, frames.back().ip - frames.back().function->bytecode.instructions.data(), line);
		if (frames.back().function->bytecode.newlines[frames.back().ip - frames.back().function->bytecode.instructions.data()]) ++line;
#endif
		switch (uint8_t instruction = *frames.back().ip++) {
		case opcode::CONSTANT:
			stack.push_back(frames.back().function->bytecode.constants[*frames.back().ip++]);
			break;
		case opcode::CONSTANT_LONG:
			{
				// TODO test this
				size_t index = read_uint16_and_update_ip(frames.back().ip);
				stack.push_back(frames.back().function->bytecode.constants[index]);
			}
			break;
		case opcode::DEFINE_GLOBAL: {
			size_t index = read_uint16_and_update_ip(frames.back().ip);
			globals[index] = stack_pop();  // TODO: consider unsigned Value
			stack.push_back(Value(true));  // TEMP - assuming this will return
			}
			break;
		case opcode::GET_GLOBAL: {
			// TODO: instead get the global at this index
			// Need "undefined" value - similar to how Python does it
			size_t index = read_uint16_and_update_ip(frames.back().ip);
			if (globals[index].type == value_type::UNINITIALIZED) {
				runtime_error("Unintialized variable", line);
				return interpret_result::RUNTIME_ERROR;
			}
			stack.push_back(globals[index]);
			}
			break;
		case opcode::GET_LOCAL: {
			size_t index = read_uint16_and_update_ip(frames.back().ip);
			// TODO: make sure this still indexes correctly
			stack.push_back(stack[frames.back().stack_index + index]);
			}
			break;
		case opcode::JUMP: {
			frames.back().ip += read_uint16_and_update_ip(frames.back().ip);
			}
			break;
		case opcode::JUMP_IF_FALSE: {
			size_t jump = read_uint16_and_update_ip(frames.back().ip);
			if (stack_peek(0).type == value_type::BOOL &&
				stack_peek(0).as.boolean == false)
				frames.back().ip += jump;
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
		case opcode::CALL: {
			size_t number_arguments = read_uint16_and_update_ip(frames.back().ip);
			if (!call(number_arguments)) return interpret_result::RUNTIME_ERROR;
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
		case opcode::RETURN: {
			Value result = stack_pop();

			if (frames.size() == 1) {
				stack_pop();
				frames.pop_back();
				std::cout << result.print() << '\n';
				return interpret_result::OK;
			}

			// TODO: verify indexing is correct here
			stack.erase(stack.begin() + frames.back().stack_index, stack.end());
			stack.push_back(result);
			frames.pop_back();
			}
			break;
		default:
			return interpret_result::RUNTIME_ERROR;
		}
	}
}