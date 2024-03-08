
#include "vm.h"
#include "compiler.h"

VirtualMachine::VirtualMachine()
{
	globals[global("cons")] = allocate(std::make_shared<BuiltinCons>(*this));
	globals[global("+")] = allocate(std::make_shared<BuiltinAdd>(*this));
	globals[global("=")] = allocate(std::make_shared<BuiltinEqual>(*this));
}

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
		uint8_t* start = frame.closure->function_ptr()->bytecode.instructions.data();
		size_t instruction = frame.ip - start - 1;
		std::cerr << "in " << frame.closure->function_ptr()->name << '\n';  // TODO: anonymous function names
	}

	frames.clear();
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

Value
VirtualMachine::allocate(std::shared_ptr<Closure> closure)
{
	memory.push_front(Data(closure));
	return Value(&memory.front());
}

Value
VirtualMachine::allocate(std::shared_ptr<BuiltinFunction> builtin)
{
	memory.push_front(Data(builtin));
	return Value(&memory.front());
}


Value
VirtualMachine::allocate(Pair pair)
{
	memory.push_front(Data(pair));
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
	Scanner scanner(source);
	Compiler compiler(scanner, *this);
	std::shared_ptr<Function> function = compiler.get_function();
	if (function == nullptr) return interpret_result::COMPILE_ERROR;

	// TODO: revisit this?
	Data script = Data(function);
	// TODO: make sure GC knows these exist
	auto closure = Data(std::make_shared<Closure>(&script));
	stack.push_back(Value(&closure));
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
	bool result = false;

	Value val = stack_peek(number_arguments);
	if (val.match_data_type(data_type::CLOSURE)) {
		std::shared_ptr<Closure> closure;
		// TODO: consider making a shortcut for this stuff
		closure = std::any_cast<std::shared_ptr<Closure>>(val.as.data->data);
		result = call(closure, number_arguments);
	}
	else if (val.match_data_type(data_type::BUILTIN)) {
		std::shared_ptr<BuiltinFunction> function;
		function = std::any_cast<std::shared_ptr<BuiltinFunction>>(val.as.data->data);
		result = call(function, number_arguments);
	}
	
	return result;
}

bool
VirtualMachine::call(std::shared_ptr<Closure> closure, size_t number_arguments)
{
	CallFrame frame{
		.closure = closure,
		.ip = &closure->function_ptr()->bytecode.instructions[0],
		.stack_index = stack.size() - number_arguments - 1 };

	frames.push_back(frame);
	return true;
}

bool
VirtualMachine::call(std::shared_ptr<BuiltinFunction> function, size_t number_arguments)
{
	std::vector<Value>::iterator iterator;
	iterator = stack.begin() + stack.size() - number_arguments;
	Value result = function->call(iterator, number_arguments);

	stack.erase(stack.begin() + stack.size() - number_arguments - 1, stack.end());
	stack.push_back(result);

	return true;
}


// TODO: clean up switch block formatting or break into functions

interpret_result
VirtualMachine::run()
{
#ifdef DEBUG_TRACE_EXECUTION
	size_t line = frames.back().closure->function_ptr()->bytecode.base_line;
#endif

	for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
		// TODO: clean this up
		disassembleInstruction(frames.back().closure->function_ptr()->bytecode, frames.back().ip - frames.back().closure->function_ptr()->bytecode.instructions.data(), line);
		if (frames.back().closure->function_ptr()->bytecode.newlines[frames.back().ip - frames.back().closure->function_ptr()->bytecode.instructions.data()]) ++line;
#endif
		switch (uint8_t instruction = *frames.back().ip++) {
		case opcode::CONSTANT:
			stack.push_back(frames.back().closure->function_ptr()->bytecode.constants[*frames.back().ip++]);
			break;
		case opcode::CONSTANT_LONG:
			{
				// TODO test this
				size_t index = read_uint16_and_update_ip(frames.back().ip);
				stack.push_back(frames.back().closure->function_ptr()->bytecode.constants[index]);
			}
			break;
		case opcode::DEFINE_GLOBAL: {
			size_t index = read_uint16_and_update_ip(frames.back().ip);
			globals[index] = stack_pop();  // TODO: consider unsigned Value
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
		case opcode::GET_UPVALUE: {
			size_t index = read_uint16_and_update_ip(frames.back().ip);
			// TODO: make sure this still indexes correctly
			stack.push_back(stack[frames.back().closure->upvalues[index].index]);
			}
			break;
		case opcode::GET_LOCAL: {
			size_t index = read_uint16_and_update_ip(frames.back().ip);
			// TODO: make sure this still indexes correctly
			stack.push_back(stack[frames.back().stack_index + index + 1]);
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
		case opcode::CALL: {
			size_t number_arguments = read_uint16_and_update_ip(frames.back().ip);
			if (!call(number_arguments)) return interpret_result::RUNTIME_ERROR;
			}
			break;
		case opcode::CLOSURE: {
			size_t index = read_uint16_and_update_ip(frames.back().ip);
			Value val = frames.back().closure->function_ptr()->bytecode.constants[index];
			if (!val.match_data_type(data_type::FUNCTION)) return interpret_result::RUNTIME_ERROR;
			auto closure = std::make_shared<Closure>(val.as.data);
			stack.push_back(allocate(closure));

			// TODO: consider getting rid of this if redundant
			size_t count = std::any_cast<std::shared_ptr<Function>>(closure->function->data)->upvalues;
			for (size_t i = 0; i < count; i++) {
				auto local = *frames.back().ip++;
				auto up_index = read_uint16_and_update_ip(frames.back().ip);

				// TEMP - always want to close off!
				RuntimeUpvalue upvalue{ .index = 0, .data = Value(false) };
				if (local) {
					upvalue = RuntimeUpvalue{
						.index = frames.back().stack_index + up_index + 1,
						// TEMP: always want to close off
						.data = stack[frames.back().stack_index + up_index + 1]
					};
				}
				else {
					upvalue = frames.back().closure->upvalues[up_index];
				}
				closure->upvalues.push_back(upvalue);
			}
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

			// Look for indexing issues here
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