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

class ServerToken: public Token {
	public:
		~ServerToken() {};
		void display() { std::cout << "{ \"" << server << " }" << std::endl; }
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
		Parser() { this->name_flag = false; };
		~Parser();
		void buildAST(std::vector<Tok*> lex);
		void displayAST();

		bool validConfiguration();
		bool conf_case1();
		bool conf_case2();

		bool validServerBlock();
		bool server_block_case1();

		bool validDirectives();
		bool dir_case1();
		bool dir_case2();

		bool validBlockDir();
		bool block_dir_case1();

		bool validSimpleDir();
		bool simple_dir_case1();
		bool simple_dir_case2();
		bool simple_dir_case3();

	private:
		bool name_flag;
		std::vector<Token*> _tokens;
		std::vector<Tok*>::iterator it;
		std::vector<Tok*> _lex;
};

#endif