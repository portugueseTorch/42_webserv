#ifndef HTTP_Lexer_HPP
# define HTTP_Lexer_HPP

# include "Webserv.hpp"

class HTTPLexer
{
    public:
        ~HTTPLexer();

        void tokenize(std::string src);
        void displayTokenList();
        const std::list<Tok> &getTokens() const;

		class emptyRequestException : public std::exception
		{
			public:
				virtual const char* what() const throw(){ return "request is empty"; }
		};
    private:
        Tok token(std::string content, TokenType type);
        std::list<Tok> _tokens;
};

#endif