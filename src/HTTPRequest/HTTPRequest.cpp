#include "HTTPRequest/HTTPRequest.hpp"


HTTPRequest::HTTPRequest()
	: _keepAlive(true), _contentLength(0), _parserCreated(false), _processed(true) {}

void HTTPRequest::process(std::string request) {
	_content = request;
	try {
		parse();
		setup();
	} catch (parserNotInitialized & e) {
		log(std::cerr, ERROR, "HTTPRequest", e.what());
	} catch(HTTPParser::invalidSyntaxException &e) {
		log(std::cerr, ERROR, "Parser", e.what());
		_processed = false;
	} catch(invalidHTTPRequest &e) {
		log(std::cerr, ERROR, "HTTPRequest", e.what());
		_processed = false;
	}
}

HTTPRequest::~HTTPRequest() {
	if (_parserCreated)
		delete parser;
};

void	HTTPRequest::parse(){
	try
	{
		HTTPLexer	lex;
		lex.tokenize(_content);
		parser = new HTTPParser(lex.getTokens());
		_parserCreated = true;
		parser->parse();
	} catch(HTTPLexer::emptyRequestException &e) {
		log(std::cerr, ERROR, "Lexer", e.what());
		throw parserNotInitialized();
	}
}

void	HTTPRequest::setup(){
	std::string headers[] = \
	{ 	"cache-control", "connection", "date", "'pragma", "trailer", \
		"transfer-encoding", "upgrade", "via", "warning", \
		"accept", "accept-charset", "accept-encoding", "accept-language", \
		"authorization", "expect", "from", "host", "if-match", "if-modified-since", \
		"if-none-match", "if-range", "if-unmodified-since", "max-forwards", \
		"proxy-authorization", "range", "referer", "referrer-policy", "te", "user-agent", \
		"allow", "content-encoding", "content-language", "content-length", \
		"content-location", "content-md5", "content-range", "content-type", \
		"expires", "last-modified", "extension-header"};
	std::vector<std::string> validHeaders(headers, headers + sizeof(headers) / sizeof(std::string));

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
			case Protocol: {
				this->_protocol = it->_content;
				break ;
			}
			case Name: {
				std::string paramName = it->_content;
				std::transform(paramName.begin(), paramName.end(), paramName.begin(), ::tolower);
				it++;
				/* The `if` statement is checking if the `paramName` is a valid header name. It does this by
				checking if the `paramName` is present in the `validHeaders` list, or if it starts with "sec-",
				or if it starts and ends with a colon. If any of these conditions are true, then the header is
				considered valid and its content is stored in the `_params` map. */
				if (std::count(validHeaders.begin(), validHeaders.end(), paramName) || \
					paramName.find("sec-") == 0 || \
					(paramName.find_first_of(':') == 0 && paramName.find_last_of(':') == paramName.size() - 1)) {
					std::string paramContent = it->_content;
					std::transform(paramContent.begin(), paramContent.end(), paramContent.begin(), ::tolower);
					checkIfConnection(paramName, paramContent);
					checkIfAccept(paramName, paramContent);
					checkContentLength(paramName, paramContent);

					_params[paramName] = paramContent;
				} else {
					//change to exception
					std::string err = "Invalid header: ";
					err.append(paramName);
					log(std::cout, ERROR, err, "");
				}
				break ;
			}
			case Body: {
				this->_body = it->_content;
				break ;
			}
			default:
				break ;
		}
	}
}

void	HTTPRequest::checkIfConnection(std::string paramName, std::string paramContent) {
	if (paramName == "connection") {
		//change to print error or throw exception
		if (paramContent != "keep-alive" && paramContent != "closed") {
			log(std::cerr, ERROR, "Connection invalid param", paramContent);
		}
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
		}
	}
}

/* 
Ja nao sei o que queria fazer com esta funcao :)
O content-length e verificado em que momento? request? response? ambos?
 */
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
		if (sep == std::string::npos) {
			log(std::cerr, ERROR, "Invalid proxy query", buf);
			throw invalidHTTPRequest();
		}
		param = buf.substr(0, sep);
		value = buf.substr(sep + 1, buf.size());
		//add exception
		if (param.size() && value.size())
			_query[param] = value;
		else {
			log(std::cerr, ERROR, "Invalid proxy query", buf);
			throw invalidHTTPRequest();
		}
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
	std::cout << std::left << std::setw(25) << "Protocol:" << std::setw(20) << _protocol << std::endl;
	std::map<std::string, std::string>::iterator it;
	
	if (_query.size()) {
		std::cout << "\n###Query params###" << std::endl;
		it = _query.begin();
		for (; it != _query.end(); it++) {
			std::string temp = it->first;
			temp.append(":");
			std::cout << std::left << std::setw(25) << temp << std::setw(20) << it->second << std::endl;
		}
		if (_accept.empty())
			std::cout << std::endl;
	}

	if (_accept.size()) {
		std::cout << "\n###Accept params###" << std::endl;
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
		std::string temp = it->first;
		temp.append(":");
		std::cout << std::left << std::setw(25) << temp << std::setw(20) << it->second << std::endl;
	}

	if (_body.size()) {
		std::cout << "\r\n" << _body << std::endl;
	}
}
