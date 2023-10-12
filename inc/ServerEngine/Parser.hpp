#pragma once
#ifndef PARSER_HPP
# define PARSER_HPP

# include "ServerEngine/Lexer.hpp"
# include "Webserv.hpp"

/*************************************/
/*************** NODES ***************/
/*************************************/

enum NodeType {
	Name,
	Parameter,
	OpenBracket,
	CloseBracket,
	Semicolon,
	ServerBlock,
	LocationBlock,
	EmptyNode,
	NLNode,
	Method,
	URI,
	Protocol,
	Body
};

// Base Node class
class Node {
	public:
		Node(std::string content, NodeType type);
		~Node();
		void display();
		std::string	_content;
		NodeType	_type;
};

//////////////////////////////////

class Parser {
	public:
		Parser(Lexer *lex);
		~Parser();

		int parse();
		void displayAST();
		const std::list<Node> &getNodes() const;

	private:
		Lexer *originalLexer;
		std::list<Node> _nodes;
		std::list<Tok>::iterator it;
		std::list<Tok> _lex;
	
		void resetNodes(int start);
		bool validSemicolon();

		bool validConfiguration();
		bool conf_case1();

		bool validServerBlock();
		bool server_block_case1();

		bool validBlock();
		bool block_case1();

		bool validDirectives();
		bool directives_case1();
		bool directives_case2();
		bool directives_case3();
		bool directives_case4();

		bool validBlockDirectives();
		bool block_directives_case1();
		bool block_directives_case2();

		bool validSimpleBlock();
		bool simple_block_case1();

		bool validSimpleDirectiveList();
		bool simple_directive_list_case1();
		bool simple_directive_list_case2();

		bool validSimpleDirective();
		bool simple_directive_case1();

		bool validParameterList();
		bool parameter_list_case1();
		bool parameter_list_case2();
};

#endif