#include "Webserv.hpp"

std::string* 	nonePointer = NULL;
std::string*	gContent = NULL;
ServerEngine* 	gServer = NULL;

int main(int argc, char **argv) {
	if (argc > 2) {
		std::cout << "Error: Usage: ./webserv [config_file_path]" << std::endl;
		return 1;
	}

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

	// Configure the servers with the information from the parser
	ServerEngine *engine = new ServerEngine(parser);
	gServer = engine;

	if (engine->configureServers()) {
		cleanUp(gContent, gServer, nonePointer);
		return 1;
	}

	// Boot servers
	if (engine->setupServers()) {
		cleanUp(gContent, gServer, nonePointer);
		return 1;
	}

	signal(SIGINT, handler);

	engine->runServers();

	// delete content and engine;
	cleanUp(gContent, gServer, nonePointer);

	return 0;
}

void handler(int signal) {
	std::cerr << std::endl;
	cleanUp(gContent, gServer, nonePointer);
	exit(signal);
}