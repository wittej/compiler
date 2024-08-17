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
	void optimize_if_tail_call(const size_t call_index) {
		for (size_t i = call_index + 3; i < instructions.size();) {
			switch (instructions[i]) {
			case opcode::RETURN:
				instructions[call_index] = opcode::TAIL_CALL;
				return;
			case opcode::JUMP: {
					uint8_t constant = instructions[i + 1];
					uint8_t overflow = instructions[i + 2];
					constexpr size_t jump = 3;
					i += static_cast<size_t>(overflow) * 256 + constant + jump;
				}
				break;
			default:
				return;
			}
		}
	}
	void tail_call_optimize() {
		for (size_t i = 0; i < instructions.size(); i++) {
			if (instructions[i] == opcode::CALL) {
				// Optimize true/false branches.
				optimize_if_tail_call(i);
			}
		}
	};
};

#endif