#include "../inc/Lexer.hpp"
#include "../inc/Parser.hpp"
#include <fstream>

int main() {
	std::ifstream in_file("test/nginx.conf");
	std::string buff;
	std::string content;

	while (std::getline(in_file, buff)) {
		content += buff;
		if (!in_file.eof())
			content += "\n";
	}

	Lexer lex;
	lex.tokenize(content);
	lex.displayTokenList();
	std::cout << std::endl;
	
	Parser parser;
	parser.buildAST(lex.getTokens());

	in_file.close();
	return 0;
}