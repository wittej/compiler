#ifndef LISP_SCANNER_H
#define LISP_SCANNER_H

#include <string>

class Scanner {
private:
	std::string& source;
	size_t start = 0;
	size_t current = 0;
	size_t line = 1;
public:
	Scanner(std::string& source) :source{ source } {};
};

#endif