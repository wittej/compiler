#include "scanner.h"

Token
Scanner::scan()
{
	while (std::isspace(source[current])) {
		if (source[current++] == '\n') ++line;
	}

	start = current;
	if (current == source.length()) return makeToken(token_type::END);

	char c = source[current++];

	// NOTE: I probably want to do something like Scheme here.
	// 1. Is it one or more non-delimiters?
	// 2. Is it a number?
	switch (c) {
	case '+':
		return makeToken(token_type::PLUS);
	case '=':
		return makeToken(token_type::EQUAL);
	}

	// TEMP - currently just numbers
	if (std::isdigit(c)) return scanNumber();
	if (std::isalpha(c)) return scanSymbol();

	return makeError("Unexpected character.");
}

Token
Scanner::scanNumber()
{
	// TODO: all kinds of identifiers
	// TODO: state machine for this
	while (std::isdigit(source[current])) ++current;
	return makeToken(token_type::NUMBER);
}

// TODO: verify this is how this type of identifier is named in Lisp.
Token
Scanner::scanSymbol()
{
	while (std::isalnum(source[current])) ++current;
	// substring check against...?
	return makeToken(symbolType());
}

token_type
Scanner::symbolType()
{
	// TODO: make these more lisp-y?
	switch (source[start]) {
	case 't':
		return checkKeyword(1, 3, "rue", token_type::TRUE);
	case 'f':
		return checkKeyword(1, 4, "alse", token_type::FALSE);
	case 'n':
		if (current > start + 1) {
			switch (source[start + 1]) {
			case 'i':
				return checkKeyword(2, 1, "l", token_type::NIL);
			case 'o':
				return checkKeyword(2, 1, "t", token_type::NOT);
			}
		}
		break;
	}

	return token_type::SYMBOL;
}

token_type
Scanner::checkKeyword(size_t index, size_t length, std::string target, token_type type)
{
	if (current - start != index + length) return token_type::SYMBOL;
	if (source.substr(start + index, length) == target) return type;
	return token_type::SYMBOL;
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