#pragma once
#ifndef PARSER_HPP
# define PARSER_HPP

#include "Lexer.hpp"

/*************************************/
/*********** DECLARE NODES ***********/
/*************************************/

// Base Node abstract class
class Token {
	public:
		virtual ~Token() {};
		virtual void display() = 0;
		TokenType type;
};

class NameToken: public Token {
	public:
		NameToken(std::string name) { this->name = name; };
		~NameToken() {};
		void display() { std::cout << "{ \"" << name << "\", " << type << " }" << std::endl; }
		std::string name;
		TokenType type = TokenType::Name;
};

class ParameterToken: public Token {
	public:
		ParameterToken(std::string param) { this->param = param; };
		~ParameterToken() {};
		void display() { std::cout << "{ \"" << param << "\", " << type << " }" << std::endl; }
		std::string param;
		TokenType type = TokenType::Parameter;
};

class PreblockToken: public Token {
	public:
		PreblockToken(std::string n, std::string p1, std::string p2) { _n = n, _p1 = p1, _p2 = p2; };
		~PreblockToken() {};
		void display() { std::cout << "{ \"[" << _n << ", " << _p1 << ", " << _p2 << "]\", " << type << " }" << std::endl; }
		std::string _n;
		std::string _p1;
		std::string _p2;
		TokenType type = TokenType::Preblock;
};

class ServerToken: public Token {
	public:
		~ServerToken() {};
		void display() { std::cout << "{ \"" << server << "\" }" << std::endl; }
		std::string server = "Server";
		TokenType type = TokenType::SBlock;
};

class OpenBracketToken: public Token {
	public:
		~OpenBracketToken() {};
		void display() { std::cout << "{ \"" << sym << "\", " << type << " }" << std::endl; }
		std::string sym = "{";
		TokenType type = TokenType::OpenBrack;
};

class CloseBracketToken: public Token {
	public:
		~CloseBracketToken() {};
		void display() { std::cout << "{ \"" << sym << "\", " << type << " }" << std::endl; }
		std::string sym = "}";
		TokenType type = TokenType::CloseBrack;
};

class SemicolonToken: public Token {
	public:
		~SemicolonToken() {};
		void display() { std::cout << "{ \"" << sym << "\", " << type << " }" << std::endl; }
		std::string sym = ";";
		TokenType type = TokenType::Semicolon;
};

//////////////////////////////////

class Parser {
	public:
		~Parser();

		void buildAST(std::vector<Tok*> lex);
		void displayAST();

		bool validConfiguration();
		bool conf_case1();
		bool conf_case2();

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
		void resetTokens(std::vector<Token*>::iterator start);
		std::vector<Token*> _tokens;
		std::vector<Tok*>::iterator it;
		std::vector<Tok*> _lex;
};

#endif