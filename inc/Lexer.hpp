#pragma once
#ifndef LEXER_HPP
# define LEXER_HPP

# include <iostream>
# include <string>
# include <list>
# include <algorithm>

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

	private:
		void trimNewLines();
		Tok token(std::string content, TokenType type);
		std::list<Tok> _tokens;
};

#endif