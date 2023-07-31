#ifndef LEXER_HPP
# define LEXER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "utils.hpp"

#define log(x) std::cout << x << std::endl

enum TokenType {
	Name,
	Parameter,
	OpenBrack,
	CloseBrack,
	Semicolon,
	SBlock,
	End
};

typedef struct Tok {
	std::string content;
	TokenType type;
} Tok;

class Lexer {
	public:
		~Lexer();
		int tokenize(std::string src);
		void displayTokenList();
		std::vector<Tok*> getTokens();
		static std::vector<std::string> directives;

	private:
		Tok *token(std::string content, TokenType type);
		std::vector<Tok*> _tokens;
};

#endif