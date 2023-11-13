#ifndef LISP_SCANNER_H
#define LISP_SCANNER_H

#include <string>

class Scanner {
private:
	std::string& source;
public:
	Scanner(std::string& source) :source{ source } {};
};

#endif