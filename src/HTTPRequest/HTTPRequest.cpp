#include "HTTPRequest/HTTPRequest.hpp"

HTTPRequest::HTTPRequest(std::string request) : _content(request){
	// try {
		parse();
		// tokenize();
		// setup();
	// } catch {
		// throw failedHTTPRequest
	// }
}

int	HTTPRequest::parse(){
	// displayRequest();
	HTTPLexer	lex;
	lex.tokenize(_content);
	lex.displayTokenList();

	
	HTTPParser parser(lex.getTokens());
	if (parser.parse())
		return 1;
	parser.displayAST();
	return 0;
}

void	HTTPRequest::displayRequest() {
	std::stringstream	cont(_content);
	std::string 		buf;

	while (getline(cont, buf, '\n')) {
		log(std::cout, INFO, buf, "");
	}
}

HTTPRequest::~HTTPRequest() {};