#include "Webserv.hpp"

std::string* 	nonePointer = NULL;
std::string*	gContent = NULL;
ServerEngine* 	gServer = NULL;

int main(int argc, char **argv) {
	// Check wrong usage
	if (argc > 2) {
		std::cout << "[ERROR]:\t\tUsage: ./webserv [config_file_path]" << std::endl;
		return 1;
	}
	try {
		gContent = new std::string(readConfigurationFile(argc, argv));
		if (!gContent)
			return 1;

		// Lex config file
		Lexer *lex = new Lexer;
		if (lex->tokenize(*gContent)) {
			cleanUp(gContent, gServer, lex);
			return 1;
		}

		// Build pseudo-AST
		Parser *parser = new Parser(lex);
		if (parser->parse()) {
			cleanUp(gContent, gServer, parser);
			return 1;
		}
		// parser.displayAST();

		// Configure the servers with the information from the parser
		ServerEngine *engine = new ServerEngine(parser);
		gServer = engine;

		if (engine->configureServers()) {
			cleanUp(gContent, gServer, nonePointer);
			return 1;
		}

		engine->displayServers();

		// Boot servers
		if (engine->setupServers()) {
			cleanUp(gContent, gServer, nonePointer);
			return 1;
		}

		// Run servers
		engine->displayServers();

		signal(SIGINT, handler);

		engine->runServers();

		// delete content and engine;
		cleanUp(gContent, gServer, nonePointer);

	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}

void handler(int signal) {
	std::cerr << std::endl;
	cleanUp(gContent, gServer, nonePointer);
	exit(signal);
}