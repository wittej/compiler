#ifndef LISP_MEMORY_H
#define LISP_MEMORY_H

#include "common.h"
#include "value.h"

struct Cons;

struct Memory {
	std::unique_ptr<Cons> next = nullptr;
};

#endif