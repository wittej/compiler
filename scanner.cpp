#include "scanner.h"

Token
Scanner::scan()
{
	while (std::isspace(source[current])) {
		if (source[current++] == '\n') ++line;
	}

	if (current == source.length()) return makeToken(token_type::END);

	start = current;
	char c = source[current++];

	// NOTE: I probably want to do something like Scheme here.
	// 1. Is it one or more non-delimiters?
	// 2. Is it a number?
	switch (c) {
	case '+':
		return makeToken(token_type::PLUS);
	}

	// TEMP - currently just numbers
	if (std::isdigit(c)) return scan_identifier();

	return makeError("Unexpected character.");
}

Token
Scanner::scan_identifier()
{
	// TODO: all kinds of identifiers
	while (std::isdigit(source[current])) ++current;
	return makeToken(token_type::NUMBER);
}

Token
Scanner::makeToken(token_type type)
{
	std::string token_string = source.substr(start, current - start);
	return Token{.type = type, .string = token_string, .line = line};
}

Token
Scanner::makeError(std::string error)
{
	return Token{ .type = token_type::ERROR, .string = error, .line = line };
}