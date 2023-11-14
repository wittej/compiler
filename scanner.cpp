#include "scanner.h"

Token
Scanner::scan()
{
	start = current;
	if (current == source.length()) return makeToken(token_type::END);
	return makeError("Unexpected character.");
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