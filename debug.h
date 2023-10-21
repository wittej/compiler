#ifndef LISP_DEBUG_H
#define LISP_DEBUG_H

#include "bytecode.h"

void disassembleBytecode(Chunk& bytecode, std::string name);
size_t disassembleInstruction(Chunk& bytecode, size_t offset);
size_t disassembleInstruction(Chunk& bytecode, size_t offset, size_t& line);

#endif