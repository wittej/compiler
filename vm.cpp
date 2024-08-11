
#include "vm.h"
#include "compiler.h"

// TODO: GC
/**
 * Construct the VM - requires allocation of built-in functions.
 */
VirtualMachine::VirtualMachine()
{
	// TODO: double-check GC handling of this - appears to be working.
	globals[global("cons")] = allocate(std::make_shared<BuiltinCons>(*this));
	globals[global("+")] = allocate(std::make_shared<BuiltinAdd>());
	globals[global("=")] = allocate(std::make_shared<BuiltinEqual>());
}

/**
 * Convenience function that pops the top value of the VM stack.
 * 
 * @return: Value popped from the stack.
 */
Value
VirtualMachine::stack_pop()
{
	Value value = stack.back();
	stack.pop_back();
	return value;
}

/**
 * Convenience function that examines a value offset from the top of the stack.
 *
 * @param depth: offset from top of stack to examine. 0 is top of stack.
 * @return: stack value offset from top by depth.
 */
Value
VirtualMachine::stack_peek(size_t depth)
{
	return stack[stack.size() - depth - 1];
}

// TODO: better line handling
/**
 * Print a runtime error with a basic stack trace and reset the stack.
 * 
 * @param message: error message to print.
 * @param line: line where error occurred (currently unused).
 */
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

// TODO: consider consolidating these - single function that takes a Data obj.

/**
 * Add a string to memory and return a Value pointing to its location.
 * 
 * @param string: string to allocate.
 * @return: Value pointing to Data in memory.
 */
Value
VirtualMachine::allocate(std::string string)
{
	return allocate(Data(string));
}

/**
 * Add a Function to memory and return a Value pointing to its location.
 *
 * @param function: shared pointer managing Function to allocate.
 * @return: Value pointing to Data in memory.
 */
Value
VirtualMachine::allocate(std::shared_ptr<Function> function)
{
	return allocate(Data(function));
}

/**
 * Add a Closure to memory and return a Value pointing to its location.
 *
 * @param closure: shared pointer managing Closure to allocate.
 * @return: Value pointing to Data in memory.
 */
Value
VirtualMachine::allocate(std::shared_ptr<Closure> closure)
{
	return allocate(Data(closure));
}

/**
 * Add a BuiltinFunction to memory and return a Value pointing to its location.
 *
 * @param builtin: shared pointer managing BuiltinFunction to allocate.
 * @return: Value pointing to Data in memory.
 */
Value
VirtualMachine::allocate(std::shared_ptr<BuiltinFunction> builtin)
{
	return allocate(Data(builtin));
}

/**
 * Add a Pair to memory and return a Value pointing to its location.
 *
 * @param pair: Pair to allocate.
 * @return: Value pointing to Data in memory.
 */
Value
VirtualMachine::allocate(Pair pair)
{
	return allocate(Data(pair));
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

// TODO: might want to just read the uint16 and move the ip in calling code.
/**
 * Convenience function that reads a uint16 in little-endian format from the
 * passed instruction pointer, incrementing it twice. NB: will update ip.
 * 
 * @param ip: pointer to first byte storing the uint16, will be updated.
 * @return: uint16 read from the ip.
 */
inline uint16_t
VirtualMachine::read_uint16_and_update_ip(uint8_t*& ip)
{
	uint8_t constant = *ip++;
	uint8_t overflow = *ip++;
	return static_cast<uint16_t>(overflow) * 256 + constant;
}

/**
 * Scan, compile, and run the source code, printing the result.
 */
interpret_result
VirtualMachine::interpret(std::string source)
{
	Scanner scanner(source);
	Compiler compiler(scanner, *this);

	auto function = compiler.compile();
	if (function == nullptr) return interpret_result::COMPILE_ERROR;

	auto script = Data(function);
	// TODO: make sure GC knows these exist
	auto closure = Data(std::make_shared<Closure>(&script));
	stack.push_back(Value(&closure));
	call(0);

	gc_active = true;
	auto result = run();
	gc_active = false;
	return result;
}

/**
 * Handles true/false status. True/false for this Lisp follows Scheme rules -
 * everything but false returns true here.
 * 
 * @param val: Value to test for truth/falsity.
 * @return: is Value not 'false'?
 */
bool
VirtualMachine::truth_value(Value val)
{
	return (val.type == value_type::BOOL) ? val.as.boolean : true;
}

// TODO: revisit error handling.

/**
 * Executes a call to a closure or a built-in function. Dispatches to
 * appropriate method for the function type.
 * 
 * @param number_arguments: number of arguments in the call.
 * @return: call success status.
 */
bool
VirtualMachine::call(size_t number_arguments)
{
	bool result = false;

	Value val = stack_peek(number_arguments);

	if (val.match_data_type(data_type::CLOSURE)) {
		using clos_t = std::shared_ptr<Closure>;
		auto closure = std::any_cast<clos_t>(val.as.data->data);
		result = call(closure, number_arguments);
	}

	else if (val.match_data_type(data_type::BUILTIN)) {
		using func_t = std::shared_ptr<BuiltinFunction>;
		auto function = std::any_cast<func_t>(val.as.data->data);
		result = call(function, number_arguments);
	}
	
	return result;
}

/**
 * Executes a call to a user-defined closure.
 *
 * @param closure: closure being called.
 * @param number_arguments: number of arguments in the call.
 * @return: call success status. (under construction)
 */
bool
VirtualMachine::call(std::shared_ptr<Closure> closure, size_t number_arguments)
{
	CallFrame frame{
		.closure = closure,
		.ip = &closure->function_ptr()->bytecode.instructions[0],
		.stack_index = stack.size() - number_arguments - 1};

	frames.push_back(frame);
	return true;  // TODO: check result?
}

/**
 * Executes a call to a built-in function.
 *
 * @param function: function being called.
 * @param number_arguments: number of arguments in the call.
 * @return: call success status. (under construction)
 */
bool
VirtualMachine::call(std::shared_ptr<BuiltinFunction> function, size_t number_arguments)
{
	std::vector<Value>::iterator iterator;
	iterator = stack.begin() + stack.size() - number_arguments;
	Value result = function->call(iterator, number_arguments);

	stack.erase(stack.begin() + stack.size() - number_arguments - 1, stack.end());
	stack.push_back(result);

	return true;  // TODO: check result?
}

/**
 * Handle upvalue capture by either creating an upvalue if none exists or
 * reusing it if it does exist. This ensures that two closures that capture the
 * same variable reference the same upvalue.
 * 
 * @param index: stack index of upvalue to capture.
 * @return: shared pointer to captured upvalue.
 */
std::shared_ptr<RuntimeUpvalue>
VirtualMachine::capture_upvalue(size_t index)
{
	auto upvalue = std::make_shared<RuntimeUpvalue>(index);

	auto start = open_upvalues.begin();
	auto end = open_upvalues.end();
	for (auto i = start; i != end; i++) {
		if ((*i)->index == upvalue->index) return *i;

		if ((*i)->index > index) {
			open_upvalues.insert(i, upvalue);
			return upvalue;
		}
	}

	open_upvalues.push_back(upvalue);
	return upvalue;
}

/**
 * Prints the current bytecode instruction - used for debugging.
 * 
 * @param line: line of code corresponding to instruction.
 */
void
VirtualMachine::disassemble_current_instruction(size_t line)
{
	auto& function = frames.back().closure->function_ptr()->bytecode;
	auto* start = function.instructions.data();
	disassembleInstruction(function, frames.back().ip - start, line);
}


// TODO: clean up switch block formatting or break into functions
// TODO: index instead of pointer?

/**
 * Execute the program. Obtains current instruction and executes it in a loop.
 * 
 * @return: result of program execution.
 */
interpret_result
VirtualMachine::run()
{

/* TODO: rewrite line calculation. Want to move off of newline system. */

#ifdef DEBUG_TRACE_EXECUTION
	size_t line = frames.back().closure->function_ptr()->bytecode.base_line;
#else
	size_t line = 0;  // TEMP
#endif

	for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
		{
			disassemble_current_instruction(line);
			// TODO: different line determination system
			auto& function = frames.back().closure->function_ptr()->bytecode;
			auto offset = frames.back().ip - function.instructions.data();
			if (function.newlines[offset]) ++line;
		};
#endif
		switch (uint8_t instruction = *frames.back().ip++) {
			case opcode::CONSTANT: {
				size_t index = read_uint16_and_update_ip(frames.back().ip);
				auto& closure = frames.back().closure;
				auto& bytecode = closure->function_ptr()->bytecode;
				stack.push_back(bytecode.constants[index]);
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
					// TODO: this isn't reporting correctly
					runtime_error("Uninitialized variable", line);
					return interpret_result::RUNTIME_ERROR;
				}
				stack.push_back(globals[index]);
				}
				break;
			case opcode::GET_UPVALUE: {
				size_t index = read_uint16_and_update_ip(frames.back().ip);
				auto upvalue = frames.back().closure->upvalues[index];
				if (upvalue->data.type != value_type::UNINITIALIZED)
					stack.push_back(upvalue->data);
				else stack.push_back(stack[upvalue->index]);
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

				// TODO: make sure this works with GC
				stack.push_back(allocate(closure));

				size_t count = std::any_cast<std::shared_ptr<Function>>(closure->function->data)->upvalues;
				for (size_t i = 0; i < count; i++) {
					auto local = *frames.back().ip++;
					auto up_index = read_uint16_and_update_ip(frames.back().ip);

					if (local) {
						auto upvalue = capture_upvalue(frames.back().stack_index + up_index + 1);
						closure->upvalues.push_back(upvalue);
					}

					else {
						auto upvalue = frames.back().closure->upvalues[up_index];
						closure->upvalues.push_back(upvalue);
					}
				}
				}
				break;
			case opcode::NOT:
				stack.push_back(Value(!truth_value(stack_pop())));
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

				// Close upvalues
				auto start = open_upvalues.rbegin();
				auto end = open_upvalues.rend();
				size_t removed = 0;
				for (auto i = start; i != end; i++) {
					if ((*i)->index < frames.back().stack_index) break;
					(*i)->data = stack[(*i)->index];
					removed++;
				}
				open_upvalues.erase(std::prev(open_upvalues.end(), removed), open_upvalues.end());

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

// TODO: different file?

Value VirtualMachine::allocate(Data object) {
#ifdef DEBUG_STRESS_GC
	if (gc_active) collect_garbage();
#endif

	memory.push_front(object);

#ifdef DEBUG_LOG_GC
	std::cerr << "ALLOCATE\n";  // TODO: printable object type.
#endif

	return Value(&memory.front());
}

void VirtualMachine::collect_garbage() {
#ifdef DEBUG_LOG_GC
	std::cerr << "-- GC BEGIN --\n";
#endif

	gc_mark();

#ifdef DEBUG_LOG_GC
	std::cerr << "-- GC END --\n";
#endif
}

// TODO: test thouroughly!
void
VirtualMachine::gc_mark()
{
	// Mark roots
	for (auto& i : stack) gc_mark(i);
	for (auto& i : globals) gc_mark(i);
	for (auto& i : frames) gc_mark(i.closure);

	// Process worklist
	while (gc_worklist.size() > 0) gc_advance_worklist();

	memory.remove_if([](auto& d) { return !d.reachable; });
	for (auto& d : memory) d.reachable = false;
}

void
VirtualMachine::gc_advance_worklist()
{
	auto next = gc_worklist.front();
	gc_worklist.pop();

	switch (next->type) {
		case data_type::UPVALUE:
			using upvalue_p = std::shared_ptr<RuntimeUpvalue>;
			gc_mark(next->cast<upvalue_p>()->data);
			break;
		case data_type::FUNCTION:
			using function_p = std::shared_ptr<Function>;
			for (auto& i : next->cast<function_p>()->bytecode.constants) {
				gc_mark(i);
			}
			break;
		case data_type::CLOSURE:
			using closure_p = std::shared_ptr<Closure>;
			gc_mark(next->cast<closure_p>()->function);
			for (auto& i : next->cast<closure_p>()->upvalues) {
				gc_mark(i->data);
			}
			break;
		default:
			break;
	}
}

void
VirtualMachine::gc_mark(Value val)
{
	if (val.type == value_type::DATA) gc_mark(val.as.data);
}

void
VirtualMachine::gc_mark(Data* data)
{
	if (data->reachable) return;

#ifdef DEBUG_LOG_GC
		std::cerr << "OBJECT MARKED\n";
#endif

	gc_worklist.push(data);
	data->reachable = true;
}

void
VirtualMachine::gc_mark(std::shared_ptr<Closure> clos)
{
	for (auto& i : clos->upvalues) {
		gc_mark(i->data);
	}
}
