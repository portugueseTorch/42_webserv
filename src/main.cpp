#include "../inc/Webserv.hpp"

int main(int argc, char **argv) {
	// Check wrong usage
	if (argc > 2) {
		std::cout << "[ERROR]:\t\tUsage: ./webserv [config_file_path]" << std::endl;
		return 1;
	}

	// try {
	// 	std::string content = readConfigurationFile(argc, argv);

	// 	// Lex config file
	// 	Lexer lex;
	// 	if (lex.tokenize(content))
	// 		return 1;

	// 	// Build pseudo-AST
	// 	Parser parser(lex.getTokens());
	// 	if (parser.parse())
	// 		return 1;
	// 	// parser.displayAST();

	// 	// Configure the servers with the information from the parser
	// 	ServerEngine engine(parser.getNodes());
	// 	if (engine.configureServers())
	// 		return 1;

	// 	// Boot servers
	// 	if (engine.setupServers())
	// 		return 1;

	// 	// Run servers
	// 	engine.displayServers();

	// } catch (std::exception &e) {
	// 	std::cerr << e.what() << std::endl;
	// 	return 1;
	// }

	HTTPRequest	req("GET / HTTP/1.1\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\nHost: www.tutorialspoint.com\r\nAccept-Language: en-us\r\nAccept-Encoding: gzip, deflate\r\nConnection: Keep-Alive");


	return 0;
}