#pragma once
#ifndef PARSER_HPP
# define PARSER_HPP

# include "Lexer.hpp"

/*************************************/
/*********** DECLARE NODES ***********/
/*************************************/

// Base Node abstract class
class Token {
	public:
		Token(std::string content, TokenType type);
		~Token();
		void display();
		std::string	_content;
		TokenType	_type;
};

// class NameToken: public Token {
// 	public:
// 		NameToken(std::string name) { this->name = name; };
// 		~NameToken() {};
// 		void display() { std::cout << "{ \"" << name << "\", " << type << " }" << std::endl; }
// 		std::string name;
// 		TokenType type = TokenType::Name;
// };

// class ParameterToken: public Token {
// 	public:
// 		ParameterToken(std::string param) { this->param = param; };
// 		~ParameterToken() {};
// 		void display() { std::cout << "{ \"" << param << "\", " << type << " }" << std::endl; }
// 		std::string param;
// 		TokenType type = TokenType::Parameter;
// };

// class PreblockToken: public Token {
// 	public:
// 		PreblockToken(std::string n, std::string p) { _n = n, _p = p; };
// 		~PreblockToken() {};
// 		void display() { std::cout << "{ \"[" << _n << ", " << _p << "]\", " << type << " }" << std::endl; }
// 		std::string _n;
// 		std::string _p;
// 		TokenType type = TokenType::Preblock;
// };

// class ServerToken: public Token {
// 	public:
// 		~ServerToken() {};
// 		void display() { std::cout << "{ \"" << server << "\" }" << std::endl; }
// 		std::string server = "Server";
// 		TokenType type = TokenType::SBlock;
// };

// class CloseBracketToken: public Token {
// 	public:
// 		~CloseBracketToken() {};
// 		void display() { std::cout << "{ \"" << sym << "\", " << type << " }" << std::endl; }
// 		std::string sym = "}";
// 		TokenType type = TokenType::CloseBrack;
// };

// class SemicolonToken: public Token {
// 	public:
// 		~SemicolonToken() {};
// 		void display() { std::cout << "{ \"" << sym << "\", " << type << " }" << std::endl; }
// 		std::string sym = ";";
// 		TokenType type = TokenType::Semicolon;
// };

//////////////////////////////////

class Parser {
	public:
		Parser(std::list<Tok> lex);
		~Parser();

		void buildAST();
		void displayAST();
		void parseAST();

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

	private:
		std::list<Token> _tokens;
		std::list<Tok>::iterator it;
		std::list<Tok> _lex;
	
		void resetTokens(int start);
		bool validSemicolon();
};

#endif