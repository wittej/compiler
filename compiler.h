#ifndef LISP_COMPILER_H
#define LISP_COMPILER_H

#include "common.h"
#include "bytecode.h"
#include "scanner.h"
#include "value.h"
#ifdef DEBUG_BYTECODE_ERRORS
#include "debug.h"
#endif
#include "vm.h"

struct Local {
	Token token;
	int depth;
	bool captured = false;
};

struct Upvalue {
	size_t index;
	bool is_local;
};

struct ParseState {
	Token current;  // Note: this is probably still important because '(' can mean different things.
	Token previous;
};

// Considering splitting this into a parser and compiler
class Compiler {
private:
	Scanner& scanner; // TODO: move to parser?
	ParseState parse;  // TODO: move features to here?
	VirtualMachine& vm;
	// TODO: consider some additional state to say what level we're at?
	// TODO: Might be covered by scope depth as well.
	std::shared_ptr<Function> function = std::make_shared<Function>();
	Compiler* enclosing = nullptr;  // This needs to be nullable - how we handle scope depth

	size_t scope_depth = 0;
	std::vector<Local> locals;
	std::vector<Upvalue> upvalues;
	bool had_error = false;  // TODO: clean this up
	bool panic_mode = false;
	void advance();
	void consume(token_type expected, std::string error_message);
	void number();
	void constant(Value value);
	void definition_or_expression();  // TODO: synchronize here
	void definition();
	void expression();
	void lambda();
	void _if();
	void error(std::string error_message, Token token);
	void write(uint8_t op);
	void parse_next();  // TODO: infix . for cons - excuse to practice parser design
	void combination();
	void _not();
	void _and();
	void _or();
	void symbol();
	void call();
	void write_uint16(uint16_t uint);
	size_t write_jump(uint8_t jump);
	void patch_jump(size_t jump_index);
	int resolve_local(Token token);
	int resolve_upvalue(Token token);
	int push_upvalue(int index, bool local);
	bool compile();
	Chunk& current_bytecode();
public:
	Compiler(Scanner& scanner, VirtualMachine& vm) : scanner{ scanner }, vm{ vm }
	{
		// TODO: revisit
		locals.push_back(Local{ .token = Token{ .type=token_type::BEGIN, .line=0 }, .depth = 0});

		compile();
	};
	Compiler(Compiler* enclosing) : enclosing{ enclosing }, scanner{ enclosing->scanner },
		vm{ enclosing->vm }, scope_depth{enclosing->scope_depth + 1} {};
	bool error() { return had_error; };
	// TODO: revisit if this is needed - might just want to make it public?
	std::shared_ptr<Function> get_function() { return had_error ? nullptr : function; };
};

#endif