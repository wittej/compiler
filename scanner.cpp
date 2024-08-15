#include "scanner.h"

/**
 * Scan source and return the next token, or a token of type token_type::END if
 * the entire source has been scanned.
 * 
 * @return: next token in the source.
 */
Token
Scanner::scan()
{
	while (std::isspace(source[current])) {
		if (source[current++] == '\n') ++line;
	}

	start = current;
	if (current == source.length()) return make_token(token_type::END);

	char c = source[current++];

	switch (c) {
		case '(':
			return make_token(token_type::LPAREN);
		case ')':
			return make_token(token_type::RPAREN);
		case '=':
			return make_token(token_type::SYMBOL);
		case '+':
		case '-':
			if (std::isdigit(source[current]) || source[current] == '.')
				return scan_number();
			else return make_token(token_type::SYMBOL);
		case '.':
			if (std::isdigit(source[current])) return scan_number();
			else return make_token(token_type::SYMBOL);
	}

	if (std::isdigit(c)) return scan_number();
	if (std::isalpha(c)) return scan_alphanumeric();

	return make_error("unexpected character '" + std::string(1, c) + "'");
}

/**
 * Used by scan() if token form indicates it is a number. Currently only
 * decimal numbers are supported, with an optional leading +/-. Increments
 * the current index past the end of the token.
 * 
 * @return: numeric token in decimal or integer format.
 */
Token
Scanner::scan_number()
{
	while (std::isdigit(source[current])) ++current;
	if (source[current] == '.') ++current;
	while (std::isdigit(source[current])) ++current;
	return make_token(token_type::NUMBER);
}

/** 
 * Used by scan when an alphanumeric token is encountered - will classify and 
 * return the token. Increments the current index past the end of the token.
 * 
 * @return: alphanumeric token.
 */
Token
Scanner::scan_alphanumeric()
{
	while (std::isalnum(source[current])) ++current;
	return make_token(classify_alphanumeric());
}

/**
 * Determines and returns the correct type for an alphanumeric token
 * encountered during scan(). 
 * 
 * @return: correct token_type for the current token being scanned.
 */
token_type
Scanner::classify_alphanumeric()
{
	switch (source[start]) {
		case 'a':
			return check_keyword(1, "nd", token_type::AND);
		case 'd':
			return check_keyword(1, "efine", token_type::DEFINE);
		case 'f':
			return check_keyword(1, "alse", token_type::FALSE);
		case 'i':
			return check_keyword(1, "f", token_type::IF);
		case 'l':
			return check_keyword(1, "ambda", token_type::LAMBDA);
		case 'n':
			if (current > start + 1) {
				switch (source[start + 1]) {
					case 'i':
						return check_keyword(2, "l", token_type::NIL);
					case 'o':
						return check_keyword(2, "t", token_type::NOT);
				}
			}
			break;
		case 'o':
			return check_keyword(1, "r", token_type::OR);
		case 's':
			return check_keyword(1, "et", token_type::SET);
		case 't':
			return check_keyword(1, "rue", token_type::TRUE);
	}

	return token_type::SYMBOL;
}

/**
 * Check the remaining characters of a string to see if it matches a specific
 * keyword (e.g., a boolean, a lambda, etc.) Return the correct type if it
 * matches or return the type for a generic symbol otherwise.
 * 
 * @param index: offset from start for the remaining characters to match. 
 * @param target: if the remaining characters match this, return 'type'.
 * @param type: the type to return if a match is found.
 * @return: type if match found, token_type::SYMBOL otherwise.
 */
token_type
Scanner::check_keyword(const size_t index, const std::string target,
					   const token_type type)
{
	if (current - start != index + target.length()) return token_type::SYMBOL;
	if (source.substr(start + index, target.length()) == target) return type;
	return token_type::SYMBOL;
}

/**
 * Return a token of the specified type given the specified source indices.
 * Expects whitespace or the end of the file after the token unless the token
 * is a left paren or is followed by a right paren, returns token with error 
 * type if whitespace condition is not met.
 * 
 * @param type: type to assign the token if it is valid.
 * @return: Token with specified type if valid, error type otherwise.
 */
Token
Scanner::make_token(const token_type type)
{
	std::string token_string = source.substr(start, current - start);

	// Require whitespace between non-parenthesis tokens.
	if (current != source.length() && !std::isspace(source[current]) &&
		source[current] != ')' && token_string != "(") {
		return make_error("unexpected characters after " + token_string + ".");
	}

	return Token{.type = type, .string = token_string, .line = line};
}

/**
* Returns a token with the specified error message.
* 
* @param error: error message to include.
* @return: Token of token_type::ERROR with specified message.
*/
Token
Scanner::make_error(const std::string error)
{
	return Token{ .type = token_type::ERROR, .string = error, .line = line };
}
