#ifndef LISP_BYTECODE_H
#define LISP_BYTECODE_H

#include "common.h"

struct Value;

// TODO: phase out CONSTANT so I consistently use uint16 for indices.

/* Using a typed enum in a namespace over an enum class here because the goal
 * of this enum is to function as a labeling system for 8-bit integer opcodes
 * which may appear in a vector with other 8-bit integers. */
namespace opcode
{
	enum opcode : uint8_t
	{
		CONSTANT, CONSTANT_LONG, TRUE, FALSE, NIL,
		ADD, NOT, EQUAL, CONS,
		DEFINE_GLOBAL, GET_GLOBAL, GET_LOCAL, GET_UPVALUE,
		RETURN, POP,
		JUMP, JUMP_IF_FALSE,
		CALL, CLOSURE,
	};
}

struct Chunk
{
private:
	size_t max_line = 0;
public:
	const size_t base_line;
	std::vector<uint8_t> instructions;
	std::vector<Value> constants;
	std::vector<bool> newlines;
	Chunk(size_t line) : base_line{ line } {};
	uint16_t add_constant(Value constant);
	void write(uint8_t op, size_t line);
	void replace(size_t index, uint8_t op);
};

#endif