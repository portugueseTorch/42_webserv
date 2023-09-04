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

        int parse();
        void displayAST();
        const std::list<Node> &getNodes() const;
    
    private:
        std::list<Node> _nodes;
        std::list<Tok>::iterator it;
        std::list<Tok> _lex;

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