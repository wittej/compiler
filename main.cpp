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

int
main(void)
{
	VirtualMachine vm;
	repl(vm);
	return 0;
}
