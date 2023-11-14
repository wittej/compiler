#ifndef LISP_SCANNER_H
#define LISP_SCANNER_H

#include <string>

enum token_type {
	ERROR, END
};

struct Token {
	token_type type;
	std::string string;
	size_t line;
};

class Scanner {
private:
	std::string& source;
	size_t start = 0;
	size_t current = 0;
	size_t line = 1;
public:
	Scanner(std::string& source) :source{ source } {};
	Token scan();
};

#endif