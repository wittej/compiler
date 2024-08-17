#ifndef LISP_BYTECODE_H
#define LISP_BYTECODE_H

#include "common.h"

struct Value;

/* Using a typed enum in a namespace over an enum class here because the goal
 * of this enum is to function as a labeling system for 8-bit integer opcodes
 * which may appear in a vector with other 8-bit integers. */
namespace opcode
{
	enum opcode : uint8_t
	{
		CONSTANT, TRUE, FALSE, NIL,
		ADD, NOT, EQUAL, CONS,
		DEFINE_GLOBAL, GET_GLOBAL, SET_GLOBAL,
		GET_LOCAL, SET_LOCAL,
		GET_UPVALUE, SET_UPVALUE,
		RETURN, POP,
		JUMP, JUMP_IF_FALSE,
		CALL, TAIL_CALL, CLOSURE,
	};
}

/**
 * Bytecode chunk with associated data to handle constant lookups, debugging.
 */
struct Chunk
{
private:
	size_t max_line = 0;
public:
	const size_t base_line;
	std::vector<uint8_t> instructions;
	std::vector<Value> constants;
	std::vector<bool> newlines;  // TODO: replace
	Chunk(size_t line) : base_line{ line } {};
	uint16_t add_constant(Value constant);
	void write(uint8_t op, size_t line);
	size_t vector_size();
	template <typename iterator>
	void optimize_if_tail_call(iterator& call) {
		call = opcode::TAIL_CALL;
	}
	void tail_call_optimize() {
		for (auto& i : instructions) {
			if (i == opcode::CALL) {
				using iterator = decltype(i);
				optimize_if_tail_call<iterator>(i);
			}
		}
	};
};

#endif