#ifndef LISP_MEMORY_H
#define LISP_MEMORY_H

#include "common.h"
#include "value.h"

struct Pair;

struct Memory {
	std::list<Pair> objects;
};

#endif