#include "HTTPRequest/Lexer.hpp"

HTTPLexer::~HTTPLexer() {
    _tokens.clear();
}

int HTTPLexer::tokenize(std::string src) {
    std::stringstream ss(src);
    std::string buf;
    std::string word;

    while (getline(ss, buf, '\n')) {
        if (buf.size() == 1)
            _tokens.push_back(token("empty", Empty));
        else {
            std::stringstream sbuf(buf);

            while (sbuf >> word) {
                if (word.find(':') == word.size() - 1) {
                    _tokens.push_back(token(word.substr(0, word.size() - 1), NameTok));
                } else {
                    _tokens.push_back(token(word, ParamTok));
                }
            }
        }
        if (ss.peek() > -1) {
            _tokens.push_back(token("nl", NL));
        } else {
            _tokens.push_back(token("EOF", End));
        }
    }
    return 0;
}

Tok HTTPLexer::token(std::string content, TokenType type) {
    Tok new_token;
    new_token.content = content;
    new_token.type = type;
    return new_token;
}

void    HTTPLexer::displayTokenList() {
    std::cout << "##### LEXER OUTPUT #####" << std::endl;
	std::list<Tok>::iterator it = _tokens.begin();
	for (; it != _tokens.end(); it++)
		std::cout << "[ \"" << it->content << "\", " << it->type << " ]" << std::endl;
}

const std::list<Tok> &HTTPLexer::getTokens() const {
	return _tokens;
}
