#ifndef LISP_SCANNER_H
#define LISP_SCANNER_H

#include "common.h"

// TODO: consider implementing logical operators as built-ins.

/* Most tokens (including most built-in functions) are symbols and will be
   resolved to values. Booleans, logical operators, numbers, and special forms
   like ternary if and lambda declaration are their own type of token. */
enum class token_type {
	BEGIN, END, ERROR,
	LPAREN, RPAREN,
	NOT, AND, OR, TRUE, FALSE,
	IF, LAMBDA,
	NUMBER, NIL, SYMBOL,
	DEFINE
};

/* Representation of a token - contains information needed for compilation and
   error reporting. */
struct Token {
	token_type type;
	std::string string;
	size_t line;
};

/* Converts source file / input line to tokens. Provides one token at a time. */
class Scanner {
private:
	std::string& source;
	size_t start = 0;
	size_t current = 0;
	size_t line = 1;
	Token makeToken(token_type type);
	Token makeError(std::string error);
	Token scan_number();
	Token scan_alphanumeric();
	token_type classify_alphanumeric();
	token_type checkKeyword(size_t index, std::string target, token_type type);
public:
	Scanner(std::string& source) :source{ source } {};
	Token scan();
};

#endif