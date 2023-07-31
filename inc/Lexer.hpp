#ifndef LEXER_HPP
# define LEXER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

enum TokenType {
	Directive,
	Parameter,
	OpenBrack,
	CloseBrack,
	Semicolon,
	End
};

typedef struct Token {
	std::string content;
	TokenType type;
} Token;

class Lexer {
	public:
		~Lexer();
		void tokenize(std::string src);
		void displayTokenList();
		static std::vector<std::string> directives;

	private:
		Token *token(std::string content, TokenType type);
		std::vector<Token*> _tokens;
};

#endif