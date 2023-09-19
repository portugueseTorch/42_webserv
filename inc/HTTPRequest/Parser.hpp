#ifndef HTTP_PARSER_HPP
# define HTTP_PARSER_HPP

# include "Webserv.hpp"
# include "HTTPRequest/Lexer.hpp"

/*************************************/
/*************** NODES ***************/
/*************************************/

class Node;

class HTTPParser {
    public:
        HTTPParser(std::list<Tok> lex);
        ~HTTPParser();

        void parse();
        void displayAST();
        const std::list<Node> &getNodes() const;

		class invalidSyntaxException : public std::exception {
			private:
				const char * _syntaxError;
			public:
				invalidSyntaxException(const char * err) : _syntaxError(err){}
				virtual const char* what() const throw() {
					return _syntaxError;
				}
		};
    
    private:
        std::list<Node> _nodes;
        std::list<Tok>::iterator it;
        std::list<Tok> _lex;
		std::string _errReason;

        void resetNodes(int start);
		bool validNL();
		bool validEmptyLine();
        bool validConfiguration();

		bool validRequestLine();
		bool validHeader();
		bool validBody();

		bool validMethod();
		bool validRequestURI();
		bool validAbsoluteURI();
		bool validHTTPVersion();

		bool validGeneralHeader();
		bool validRequestHeader();
		bool validEntityHeader();
};

#endif