#ifndef HTTP_Lexer_HPP
# define HTTP_Lexer_HPP

# include "Webserv.hpp"
# include "HTTPRequest/utils.hpp" 

class HTTPLexer
{
    public:
        ~HTTPLexer();

        int tokenize(std::string src);
        void displayTokenList();
        const std::list<Tok> &getTokens() const;
    private:
        Tok token(std::string content, TokenType type);
        std::list<Tok> _tokens;
};

#endif