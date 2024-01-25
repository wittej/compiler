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
};

// Considering splitting this into a parser and compiler
class Compiler {
private:
	Scanner scanner; // Encapsulates source - probably don't need to work with it directly
	VirtualMachine& vm;
	// TODO: consider some additional state to say what level we're at?
	// TODO: Might be covered by scope depth as well.
	std::shared_ptr<Function> function = std::make_shared<Function>();
	// TODO: abstract these 
	Token parse_current;  // Note: this is probably still important because '(' can mean different things.
	Token parse_previous;
	size_t scope_depth = 0;
	std::vector<Local> locals;
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
	void if_statement();
	void error(std::string error_message, Token token);
	void write(uint8_t op);
	void parse();  // TODO: infix . for cons - excuse to practice parser design
	void combination();
	void temp_add();
	void temp_not();
	void temp_and();
	void temp_or();
	void temp_equal();
	void temp_cons();
	void temp_let();
	void symbol();
	void call();
	void write_uint(uint16_t uint);
	size_t write_jump(opcode::opcode jump);
	void patch_jump(size_t jump_index);
	int resolve_local(Token token);
	bool compile();
	Chunk& current_bytecode();
public:
	Compiler(std::string& source, VirtualMachine& vm) : scanner{ Scanner(source) }, vm{ vm }
	{
		// TODO: revisit
		locals.push_back(Local{ .token = Token{ .type=token_type::BEGIN, .line=0 }, .depth = 0});

		compile();
	};
	bool error() { return had_error; };
	// TODO: revisit if this is needed - might just want to make it public?
	std::shared_ptr<Function> get_function() { return had_error ? nullptr : function; };
	Chunk get_bytecode() { return current_bytecode(); };
};

#endif