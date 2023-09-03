#include "HTTPRequest/HTTPRequest.hpp"


HTTPRequest::HTTPRequest(std::string request)
	: _content(request), _keepAlive(true), _contentLength(0)
	{
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
	std::vector<std::string> validHeaders = \
	{ 	"cache-control", "connection", "date", "'pragma", "trailer", \
		"transfer-encoding", "upgrade", "via", "warning", \
		"accept", "accept-charset", "accept-encoding", "accept-language", \
		"authorization", "expect", "from", "host", "if-match", "if-modified-since", \
		"if-none-match", "if-range", "if-unmodified-since", "max-forwards", \
		"proxy-authorization", "range", "referer", "te", "user-agent", \
		"allow", "content-encoding", "content-language", "content-length", \
		"content-location", "content-md5", "content-range", "content-type", \
		"expires", "last-modified", "extension-header"};

	std::list<Node>::const_iterator it = parser->getNodes().begin();

	for (; it != parser->getNodes().end(); it++) {
		switch (it->_type) {
			case Method: {
				this->_method = it->_content;
				break ;
			}
			case URI: {
				if (it->_content.find(".py") != std::string::npos) {
					this->_requestURI = it->_content.substr(0, it->_content.find('?'));
					extractQuery(it->_content);
				} else {
					this->_requestURI = it->_content;
				}
				break ;
			}
			case Name: {
				std::string paramName = it->_content;
				std::transform(paramName.begin(), paramName.end(), paramName.begin(), ::tolower);
				it++;
				if (std::count(validHeaders.begin(), validHeaders.end(), paramName)) {
					std::string paramContent = it->_content;
					std::transform(paramContent.begin(), paramContent.end(), paramContent.begin(), ::tolower);
					checkIfConnection(paramName, paramContent);
					checkIfAccept(paramName, paramContent);
					checkContentLength(paramName, paramContent);

					// _keepAlive = paramName == 'keep-alive' ? it->_content == 'true' : false;
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

void	HTTPRequest::checkIfConnection(std::string paramName, std::string paramContent) {
	if (paramName == "connection") {
		//change to print error or throw exception
		if (paramContent != "keep-alive" && paramContent != "closed")
			;
		_keepAlive = (paramContent == "keep-alive") ? true : false;
	}
}

/* Parsing is messed up, what is really needed? */
void	HTTPRequest::checkIfAccept(std::string paramName, std::string paramContent) {
	if (paramName == "accept") {
		std::stringstream ss(paramContent);
		std::string buf;
		std::string param;
		std::string value;

		while (getline(ss, buf, ',')) {
			size_t noSpace = buf.find_first_not_of(" \t\v");
			buf.erase(0, noSpace);
			size_t sep = buf.find('/');
			//change to if !sep throw exception
			if (sep == std::string::npos)
				continue ;
			param = buf.substr(0, sep);
			value = buf.substr(sep + 1, buf.size());
			//add exception
			if (param.size() && value.size())
				_accept[param] = value;
			// std::cout << param << ", " << value << std::endl;
			// std::cout << buf << std::endl;
		}
	}
}

void	HTTPRequest::checkContentLength(std::string paramName, std::string paramContent) {
	if (paramName == "content-length") {
		//change to print error or throw exception
		_contentLength = std::atoi(paramContent.c_str());
	}
}

void	HTTPRequest::extractQuery(std::string URI) {
	std::string query = URI.substr(URI.find('?') + 1, URI.size());
	std::stringstream squery(query);
	std::string buf;
	std::string param;
	std::string value;

	while (getline(squery, buf, '&')) {
		size_t sep = buf.find('=');
		//change to if !sep throw exception
		if (sep == std::string::npos)
			continue ;
		param = buf.substr(0, sep);
		value = buf.substr(sep + 1, buf.size());
		//add exception
		if (param.size() && value.size())
			_query[param] = value;
		// std::cout << param << ", " << value << std::endl;
	}
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
	std::map<std::string, std::string>::iterator it;
	
	if (_query.size()) {
		std::cout << "\n###Query params###" << std::endl;
		it = _query.begin();
		for (; it != _query.end(); it++) {
		std::string temp = it->first;
		temp.append(":");
		std::cout << std::left << std::setw(25) << temp << std::setw(20) << it->second << std::endl;
		}
		std::cout << std::endl;
	}

	if (_accept.size()) {
		std::cout << "###Accept params###" << std::endl;
		it = _accept.begin();
		for (; it != _accept.end(); it++) {
		std::string temp = it->first;
		temp.append(":");
		std::cout << std::left << std::setw(25) << temp << std::setw(20) << it->second << std::endl;
		}
		std::cout << std::endl;
	}

	std::cout << std::left << std::setw(25) << "keep-alive:" << 
		std::setw(20) << (_keepAlive ? "true" : "false")<< std::endl << std::endl;

	std::cout << std::left << std::setw(25) << "content-length" << 
		std::setw(20) << _contentLength << std::endl << std::endl;

	it = _params.begin();
	for (; it != _params.end(); it++) {
		// if (it->first == "connection")
		// 	continue ;
		std::string temp = it->first;
		temp.append(":");
		std::cout << std::left << std::setw(25) << temp << std::setw(20) << it->second << std::endl;
	}

}