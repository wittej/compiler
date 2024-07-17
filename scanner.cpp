#include "scanner.h"

/* Scan source and return the next token, or a token of type token_type::END if
*  the entire source has been scanned.
* 
*  :return: next token in the source.
*/
Token
Scanner::scan()
{
	while (std::isspace(source[current])) {
		if (source[current++] == '\n') ++line;
	}

	start = current;
	if (current == source.length()) return makeToken(token_type::END);

	char c = source[current++];

	switch (c) {
		case '(':
			return makeToken(token_type::LPAREN);
		case ')':
			return makeToken(token_type::RPAREN);
		case '=':
			return makeToken(token_type::SYMBOL);
		case '+':
		case '-':
			if (std::isdigit(source[current]) || source[current] == '.')
				return scan_number();
			else return makeToken(token_type::SYMBOL);
		case '.':
			if (std::isdigit(source[current])) return scan_number();
			else return makeToken(token_type::SYMBOL);
	}

	if (std::isdigit(c)) return scan_number();
	if (std::isalpha(c)) return scan_alphanumeric();

	return makeError("unexpected character '" + std::string(1, c) + "'");
}

/* Used by scan() if token form indicates it is a number. Currently only
*  decimal numbers are supported, with an optional leading +/-. Increments
*  the current index past the end of the token.
* 
*  :return: numeric token in decimal or integer format.
*/
Token
Scanner::scan_number()
{
	while (std::isdigit(source[current])) ++current;
	if (source[current] == '.') ++current;
	while (std::isdigit(source[current])) ++current;
	return makeToken(token_type::NUMBER);
}

/* Used by scan when an alphanumeric token is encountered - will classify
*  and return the token. Increments the current index past the end of the
*  token.
* 
*  :return: alphanumeric token.
*/
Token
Scanner::scan_alphanumeric()
{
	while (std::isalnum(source[current])) ++current;
	return makeToken(classify_alphanumeric());
}

/* Determines and returns the correct type for an alphanumeric token
*  encountered during scan(). 
* 
*  :return: correct token_type for the current token being scanned.
*/
token_type
Scanner::classify_alphanumeric()
{
	switch (source[start]) {
		case 'a':
			return checkKeyword(1, "nd", token_type::AND);
		case 'd':
			return checkKeyword(1, "efine", token_type::DEFINE);
		case 'f':
			return checkKeyword(1, "alse", token_type::FALSE);
		case 'i':
			return checkKeyword(1, "f", token_type::IF);
		case 'l':
			return checkKeyword(1, "ambda", token_type::LAMBDA);
		case 'n':
			if (current > start + 1) {
				switch (source[start + 1]) {
					case 'i':
						return checkKeyword(2, "l", token_type::NIL);
					case 'o':
						return checkKeyword(2, "t", token_type::NOT);
				}
			}
			break;
		case 'o':
			return checkKeyword(1, "r", token_type::OR);
		case 't':
			return checkKeyword(1, "rue", token_type::TRUE);
	}

	return token_type::SYMBOL;
}

token_type
Scanner::checkKeyword(const size_t index, const std::string target,
					  const token_type type)
{
	if (current - start != index + target.length()) return token_type::SYMBOL;
	if (source.substr(start + index, target.length()) == target) return type;
	return token_type::SYMBOL;
}

Token
Scanner::makeToken(const token_type type)
{
	std::string token_string = source.substr(start, current - start);

	// Require whitespace between non-parenthesis tokens.
	if (current != source.length() && !std::isspace(source[current]) &&
		source[current] != ')' && token_string != "(") {
		return makeError("expected end of token " + token_string + ".");
	}

	return Token{.type = type, .string = token_string, .line = line};
}

Token
Scanner::makeError(const std::string error)
{
	return Token{ .type = token_type::ERROR, .string = error, .line = line };
}
