#include "../inc/Lexer.hpp"

int isindent(int i) {
	return (i == ' ' || i == '\n' || i == '\t');
}

std::vector<std::string> Lexer::directives = {
	"error_page",
	"index",
	"listen",
	"location"
	"root",
	"server",
	"server_name",
	"http",
};

Lexer::~Lexer() {
	std::vector<Token*>::iterator it = _tokens.begin();
	for (; it != _tokens.end(); it++)
		delete *it;
	_tokens.clear();
}

void Lexer::tokenize(std::string src) {
	for (int i = 0; src[i]; i++) {
		if (isindent(src[i]))
			continue;
		else if (src[i] == '{')
			_tokens.push_back(token("{", OpenBrack));
		else if (src[i] == '}')
			_tokens.push_back(token("}", CloseBrack));
		else if (src[i] == ';')
			_tokens.push_back(token(";", Semicolon));
		else {
			if (src[i] == '#') {
				while (src[i] && src[i] != '\n')
					i++;
			} else {
				int start = i;
				while (src[i] && !isindent(src[i]) && (src[i] != ';' && src[i] != '{' && src[i] != '}'))
					i++;
				std::string term = src.substr(start, i - start);
				if (std::find(directives.begin(), directives.end(), term) != directives.end())
					_tokens.push_back(token(term, Directive));
				else
					_tokens.push_back(token(term, Parameter));
				if (src[i] == ';' || src[i] == '{' || src[i] == '}')
					i--;
			}
		}
	}
	_tokens.push_back(token("EOF", End));
}

Token *Lexer::token(std::string content, TokenType type) {
	Token *new_token = new Token;
	new_token->content = content;
	new_token->type = type;
	return (new_token);
}

void Lexer::displayTokenList() {
	std::vector<Token*>::iterator it = _tokens.begin();
	for (; it != _tokens.end(); it++)
		std::cout << "{ \"" << (*it)->content << "\", " << (*it)->type << " }" << std::endl;
}
