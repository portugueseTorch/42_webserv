#include "../inc/Lexer.hpp"
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
	std::cout << "\n\n" << std::endl;
	lex.displayTokenList();

	in_file.close();
	return 0;
}