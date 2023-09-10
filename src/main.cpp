#include "Webserv.hpp"

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

	//will delete later
	(void)argv;
	HTTPRequest req;
	req.process("POST /admin/login.py?user=user&password=pass HTTP/1.1\r\n \
		User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n \
		Host: www.tutorialspoint.com\r\n \
		Accept: */*\r\n \
		sec-ch-ua: \"Not.A/Brand\";v=\"8\", \"Chromium\";v=\"114\", \"Brave\";v=\"114\"\r\n \
		Accept-Language: en-us\r\n \
		Accept-Encoding: gzip, deflate\r\n \
		:Authority:: www.google.com\r\n \
		Accept: text/html, application/xhtml+xml, application/xml;q=0.9, image/webp, */*;q=0.8\r\n \
		Connection: keep-alive\r\n\r\n \
		<h1>Header</h1>\r\n\r\n");
	if (req.success())
		req.displayParsedRequest();

	return 0;
}
