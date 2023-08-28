#ifndef HTTP_PARSER_HPP
# define HTTP_PARSER_HPP

# include "Webserv.hpp"
# include "HTTPRequest/Lexer.hpp"
# include "HTTPRequest/utils.hpp" 


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
        bool validConfiguration();

        bool conf_case1();
        bool conf_case2();
        bool conf_case3();
};

#endif