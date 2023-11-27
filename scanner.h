#ifndef LISP_SCANNER_H
#define LISP_SCANNER_H

#include "common.h"

enum class token_type {
	END, ERROR,
	PLUS, NOT, EQUAL, CONS, // TEMP - for testing purposes
	NUMBER, TRUE, FALSE, NIL, SYMBOL,
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
	Token makeToken(token_type type);
	Token makeError(std::string error);
	Token scanNumber();
	Token scanSymbol();
	token_type symbolType();
	token_type checkKeyword(size_t index, std::string target, token_type type);
public:
	Scanner(std::string& source) :source{ source } {};
	Token scan();
};

#endif