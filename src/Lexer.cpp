#include "../inc/Lexer.hpp"

std::vector<std::string> Lexer::directives = {
	"error_page",
	"index",
	"listen",
	"location"
	"root",
	"server",
	"server_name",
};

Lexer::~Lexer() {
	std::vector<Tok*>::iterator it = _tokens.begin();
	for (; it != _tokens.end(); it++)
		delete *it;
	_tokens.clear();
}

int Lexer::tokenize(std::string src) {
	try {
		for (int i = 0; src[i]; i++) {
			if (isindent(src[i]))
				continue;
			else if (src[i] == '{')
				_tokens.push_back(token("{", TokenType::OpenBrack));
			else if (src[i] == '}')
				_tokens.push_back(token("}", TokenType::CloseBrack));
			else if (src[i] == ';')
				_tokens.push_back(token(";", TokenType::Semicolon));
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
						_tokens.push_back(token(term, TokenType::Name));
					else
						_tokens.push_back(token(term, TokenType::Parameter));
					if (src[i] == ';' || src[i] == '{' || src[i] == '}')
						i--;
				}
			}
		}
		_tokens.push_back(token("EOF", End));
	} catch (std::exception &e) {
		std::cerr << "FATAL: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}

Tok *Lexer::token(std::string content, TokenType type) {
	Tok *new_token = new Tok;
	new_token->content = content;
	new_token->type = type;
	return new_token;
}

void Lexer::displayTokenList() {
	std::cout << "##### LEXER OUTPUT #####" << std::endl;
	std::vector<Tok*>::iterator it = _tokens.begin();
	for (; it != _tokens.end(); it++)
		std::cout << "{ \"" << (*it)->content << "\", " << (*it)->type << " }" << std::endl;
}

std::vector<Tok*> Lexer::getTokens() {
	return _tokens;
}
