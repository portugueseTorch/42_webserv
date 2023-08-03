#include "../inc/Lexer.hpp"

std::list<std::string> Lexer::directives = {
	"error_page",
	"index",
	"listen",
	"location"
	"root",
	"server",
	"server_name",
};

Lexer::~Lexer() {
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
				_tokens.push_back(token(";", TokenType::SemiTok));
			else if (src[i] == '\n')
				_tokens.push_back(token("nl", TokenType::NL));
			else {
				if (src[i] == '#') {
					while (src[i] && src[i] != '\n')
						i++;
					if (src[i] && src[i] == '\n')
						_tokens.push_back(token("nl", TokenType::NL));
				} else {
					int start = i;
					while (src[i] && !isindent(src[i]) && (src[i] != ';' && src[i] != '{' && src[i] != '}' && src[i] != '\n'))
						i++;
					std::string term = src.substr(start, i - start);
					if (std::find(directives.begin(), directives.end(), term) != directives.end())
						_tokens.push_back(token(term, TokenType::NameTok));
					else
						_tokens.push_back(token(term, TokenType::ParamTok));
					if (src[i] == ';' || src[i] == '{' || src[i] == '}' || src[i] == '\n')
						i--;
				}
			}
		}
		_tokens.push_back(token("EOF", End));
	} catch (std::exception &e) {
		std::cerr << "FATAL: " << e.what() << std::endl;
		return 1;
	}
	trimNewLines();
	return 0;
}

Tok Lexer::token(std::string content, TokenType type) {
	Tok new_token;
	new_token.content = content;
	new_token.type = type;
	return new_token;
}

void Lexer::displayTokenList() {
	std::cout << "##### LEXER OUTPUT #####" << std::endl;
	std::list<Tok>::iterator it = _tokens.begin();
	for (; it != _tokens.end(); it++)
		std::cout << "[ \"" << it->content << "\", " << it->type << " ]" << std::endl;
}

void Lexer::trimNewLines() {
	std::list<Tok>::iterator it = _tokens.begin();
	for (; it != _tokens.end(); it++) {
		if (it->type == TokenType::NL) {
			it++;
			while (it != _tokens.end() && it->type == TokenType::NL)
				_tokens.erase(it++);
			it--;
		}
	}
}

const std::list<Tok> &Lexer::getTokens() const {
	return _tokens;
}
