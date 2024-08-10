#include "common.h"
#include "bytecode.h"
#include "debug.h"
#include "vm.h"


/*
TODOs:

Better reporting for compilation inside function
Add new features to test file as they are implemented
Recursion
Error type
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
