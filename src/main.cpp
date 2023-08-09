#include "../inc/Webserv.hpp"

int main(int argc, char **argv) {
	// Check wrong usage
	if (argc > 2) {
		std::cout << "[ERROR]:\t\tUsage: ./webserv [config_file_path]" << std::endl;
		return 1;
	}

	try {
		std::string content = readConfigurationFile(argc, argv);

		// Lex config file
		Lexer lex;
		if (lex.tokenize(content))
			return 1;

		// Build pseudo-AST
		Parser parser(lex.getTokens());
		if (parser.buildAST())
			return 1;
		parser.displayAST();

		ServerEngine engine(parser.getNodes());
		if (engine.configureServers())
			return 1;
		engine.displayServers();

	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}