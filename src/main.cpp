#include "Webserv.hpp"

int main(int argc, char **argv) {
	if (argc > 2) {
		std::cout << "Error: Usage: ./webserv [config_file_path]" << std::endl;
		return 1;
	}

	try {
		std::string content = readConfigurationFile(argc, argv);

		Lexer lex;
		if (lex.tokenize(content))
			return 1;

		Parser parser(lex.getTokens());
		if (parser.parse())
			return 1;

		ServerEngine engine(parser.getNodes());
		if (engine.configureServers())
			return 1;

		if (engine.setupServers())
			return 1;

		engine.runServers();

	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
