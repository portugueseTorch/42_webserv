#include "Webserv.hpp"

Lexer::~Lexer() {
	_tokens.clear();
}

/**
 * @brief Consumes the contents of the configuration file in the form of a string
 * and populates the [_tokens] list with Tokens of type TokenType. The resulting
 * token list contains new_line tokens, which get removed in the last step of lexing
 * 
 * @param src Contents of the configuration file
 * @return 0 on success, 1 on failure
 */
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
			else if (src[i] == '\'') {
				log(std::cerr, MsgType::ERROR, "Invalid .conf syntax", "\'");
				return 1;
			}
			else if (src[i] == '\"') {
				i++;
				std::string param = "";
				while (src[i] && src[i] != '\"') {
					param += src[i];
					i++;
				}
				_tokens.push_back(token(param, TokenType::ParamTok));
			} else {
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
					if (std::find(ServerEngine::directives.begin(), ServerEngine::directives.end(), term) != ServerEngine::directives.end())
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

/**
 * @brief Creates a new token on the stack with content CONTENT and 
 * type TYPE
 * 
 * @param content string with the content to populate the Token
 * @param type type of the token from TokenType
 * @return created token 
 */
Tok Lexer::token(std::string content, TokenType type) {
	Tok new_token;
	new_token.content = content;
	new_token.type = type;
	return new_token;
}

/**
 * @brief Displays [_tokens]
 * 
 */
void Lexer::displayTokenList() {
	std::cout << "##### LEXER OUTPUT #####" << std::endl;
	std::list<Tok>::iterator it = _tokens.begin();
	for (; it != _tokens.end(); it++)
		std::cout << "[ \"" << it->content << "\", " << it->type << " ]" << std::endl;
}

/**
 * @brief Removes the new line tokens from the _tokens list as the last
 * step of the lexing process
 * 
 */
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

/**
 * @brief Getter for the [_tokens] list, returning a constant reference to it
 * 
 * @return const std::list<Tok>& 
 */
const std::list<Tok> &Lexer::getTokens() const {
	return _tokens;
}
