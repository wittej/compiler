#ifndef LISP_DEBUG_H
#define LISP_DEBUG_H

#include "bytecode.h"

void disassembleBytecode(std::vector<opcode>& bytecode, std::string name);
size_t disassembleInstruction(std::vector<opcode>& bytecode, size_t offset);

#endif