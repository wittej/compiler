#include "common.h"
#include "bytecode.h"
#include "debug.h"
#include "vm.h"


/*
TODOs:

Better reporting for compilation inside function - do during the new static
	analysis step.

Recursion - tail recursion basic idea: return and evaluate a tail call as
	planned. However, this is pretty wasteful if it's not actually in a
	"tail context". Whether a call is in a tail context can be determined
	with static analysis in a post-processing step. (Read from bytecode -
	see if it jumps to return. If so, it's a tail call.) Tail call is like
	a regular call except we clean up the prior stack frame first.

Add new features to test file as they are implemented
	- set in all contexts.

Error type

Refactor memory out of VM.

Refactor parser / compiler?
*/


constexpr size_t LINE_SIZE = 1024;

// TODO: method?
void
repl(VirtualMachine& vm)
{
	std::string line;
	for (;;) {
		std::cout << "> ";

		if (!std::getline(std::cin, line)) break;

		switch (vm.interpret(line)) {
			case interpret_result::COMPILE_ERROR:
				std::cout << "COMPILER ERROR\n";
				break;
			case interpret_result::RUNTIME_ERROR:
				std::cout << "RUNTIME ERROR\n";
				break;
		}
	}
}

// TODO: runfile
// NB: temporarily simulating this using the shell

int
main(void)
{
	VirtualMachine vm;
	repl(vm);
	return 0;
}
