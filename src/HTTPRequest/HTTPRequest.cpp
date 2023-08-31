#include "HTTPRequest/HTTPRequest.hpp"


HTTPRequest::HTTPRequest(std::string request) : _content(request){
	// try {
		parse();
		// tokenize();
		setup();
		displayParsedRequest();
	// } catch {
		// throw failedHTTPRequest
	// }
}

HTTPRequest::~HTTPRequest() {
	delete parser;
};

int	HTTPRequest::parse(){
	// displayRequest();
	HTTPLexer	lex;
	lex.tokenize(_content);
	lex.displayTokenList();

	// parser->setLexer(lex.getTokens());
	parser = new HTTPParser(lex.getTokens());
	// HTTPParser parser(lex.getTokens());
	if (parser->parse())
		return 1;
	parser->displayAST();
	// this->it = parser.getNodes().begin();
	// this->end = parser.getNodes().end();
	return 0;
}

int	HTTPRequest::setup(){
	std::vector<std::string> validHeaders = { "host", "user-agent", "accept-language", "accept-encoding", "connection" };

	std::list<Node>::const_iterator it = parser->getNodes().begin();

	for (; it != parser->getNodes().end(); it++) {
		switch (it->_type) {
			case Method: {
				this->_method = it->_content;
				break ;
			}
			case URI: {
				this->_requestURI = it->_content;
				break ;
			}
			case Name: {
				std::string paramName = it->_content;
				std::transform(paramName.begin(), paramName.end(), paramName.begin(), ::tolower);
				it++;
				if (std::count(validHeaders.begin(), validHeaders.end(), paramName)) {
					std::string paramContent = it->_content;
					_params[paramName] = paramContent;
				} else {
					std::string err = "Invalid header: ";
					err.append(paramName);
					log(std::cout, ERROR, err, "");
				}
				break ;
			}
			default:
				// std::cout << it->_type << ", " << it->_content << std::endl;
				break ;
		}
	}
	return 0;
}

void	HTTPRequest::displayRequest() {
	std::stringstream	cont(_content);
	std::string 		buf;

	while (getline(cont, buf, '\n')) {
		log(std::cout, INFO, buf, "");
	}
}

void HTTPRequest::displayParsedRequest(){
	std::cout << std::left << std::setw(25) << "Method: " << std::setw(20) << _method << std::endl;
	std::cout << std::left << std::setw(25) << "Request URI:" << std::setw(20) << _requestURI << std::endl;
	// std::cout << "Params:" << std::endl;
	std::map<std::string, std::string>::iterator it = _params.begin();
	for (; it != _params.end(); it++) {
		std::string temp = it->first;
		temp.append(":");
		std::cout << std::left << std::setw(25) << temp << std::setw(20) << it->second << std::endl;
	}
}