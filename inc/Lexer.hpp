#pragma once
#ifndef LEXER_HPP
# define LEXER_HPP

# include <iostream>
# include <string>
# include <list>
# include <algorithm>
# include "utils.hpp"

# define log(x) std::cout << x << std::endl

enum TokenType {
	NameTok,
	ParamTok,
	OpenBrack,
	CloseBrack,
	SemiTok,
	SBlock,
	Preblock,
	NL,
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
		const std::list<Tok> &getTokens() const;
		static std::list<std::string> directives;

	private:
		void trimNewLines();
		Tok token(std::string content, TokenType type);
		std::list<Tok> _tokens;
};

#endif