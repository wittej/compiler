#include "common.h"
#include "bytecode.h"
#include "debug.h"
#include "vm.h"


#define LINE_SIZE 1024


void
repl(VirtualMachine vm)
{
	std::string line;
	for (;;) {
		std::cout << "> ";
		if (!std::getline(std::cin, line)) break;
		vm.interpret(line);
	}
}

// TODO: runfile

int
main(void)
{
	VirtualMachine vm;
	repl(vm);
	return 0;
}
