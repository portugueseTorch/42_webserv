#include "HTTPRequest/HTTPRequest.hpp"


HTTPRequest::HTTPRequest():
	_statusCode(200),
	_keepAlive(true),
	_contentLength(0),
	_parserCreated(false) {
	_port = htons(8080);
	_ip_address = inet_addr("0.0.0.0");
}

void HTTPRequest::process(std::string request) {
	_content = request;
	try {
		parse();
		setup();
		log(std::cout, SUCCESS, "HTTP Request successfully processed", "");
	} catch (parserNotInitialized &e) {
		_statusCode = 400;
		log(std::cerr, ERROR, "HTTPRequest", e.what());
	} catch(HTTPParser::invalidSyntaxException &e) {
		log(std::cerr, ERROR, "HTTP Parser", e.what());

		std::string err = e.what();
		if (err == "wrong HTTP request method")
			_statusCode = 405;
		else
			_statusCode = 400;
	} catch(invalidHTTPRequest &e) {
		_statusCode = 400;
	}
}

HTTPRequest::~HTTPRequest() {
	if (_parserCreated)
		delete parser;
};

void	HTTPRequest::setPort(uint32_t port) {
	_port = port;
}

void	HTTPRequest::setIPAddress(in_addr_t ip_address) {
	_ip_address = ip_address;
}

std::string	HTTPRequest::getServerName() {
	std::string host;
	std::vector<std::string> split;

	if (getAllParams().count("host") == 0)
		return "";
	host = getAllParams()["host"];
	// Split param
	int end = host.find(":"); 
	while (end != -1) { // Loop until no delimiter is left in the string.
		split.push_back(host.substr(0, end));
		host.erase(host.begin(), host.begin() + end + 1);
		end = host.find(":");
	}
	split.push_back(host.substr(0, end));

	return split[0];
}

void	HTTPRequest::parse(){
	try
	{
		HTTPLexer	lex;
		lex.tokenize(_content);
		parser = new HTTPParser(lex.getTokens());
		_parserCreated = true;
		parser->parse();
	} catch(HTTPLexer::emptyRequestException &e) {
		log(std::cerr, ERROR, "HTTPLexer", e.what());
		throw parserNotInitialized();
	}
}

void	HTTPRequest::setup() {
	std::string headers[] = \
	{ 	"cache-control", "connection", "date", "'pragma", "trailer", \
		"transfer-encoding", "upgrade", "via", "warning", \
		"accept", "accept-charset", "accept-encoding", "accept-language", \
		"authorization", "expect", "from", "host", "if-match", "if-modified-since", \
		"if-none-match", "if-range", "if-unmodified-since", "max-forwards", \
		"proxy-authorization", "range", "referer", "referrer-policy", "te", "user-agent", \
		"allow", "content-encoding", "content-language", "content-length", \
		"content-location", "content-md5", "content-range", "content-type", \
		"expires", "last-modified", "extension-header", "upgrade-insecure-requests" };
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
					if (it->_content != "/" && it->_content[it->_content.length() - 1] == '/')
						_requestURI = it->_content.substr(0, it->_content.length() - 1);
					else
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
					checkContentLength(paramName, paramContent);

					_params[paramName] = paramContent;
				} else {
					std::string err = "Invalid header: ";
					err.append(paramName);
					log(std::cout, ERROR, err, "");
					throw invalidHTTPRequest();
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
		if (paramContent != "keep-alive" && paramContent != "closed") {
			log(std::cerr, ERROR, "Connection invalid param", paramContent);
			throw invalidHTTPRequest();
		}
		_keepAlive = (paramContent == "keep-alive") ? true : false;
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
	std::cout << std::left << std::setw(25) << "Host: " << _ip_address << ":" << ntohs(_port) << std::endl;
	std::cout << std::left << std::setw(25) << "Method: " << _method << std::endl;
	std::cout << std::left << std::setw(25) << "Request URI:" << _requestURI << std::endl;
	std::cout << std::left << std::setw(25) << "Protocol:" << _protocol << std::endl;
	std::map<std::string, std::string>::iterator it;
	
	if (_query.size()) {
		std::cout << "\n###Query params###" << std::endl;
		it = _query.begin();
		for (; it != _query.end(); it++) {
			std::string temp = it->first;
			temp.append(":");
			std::cout << std::left << std::setw(25) << temp << std::setw(20) << it->second << std::endl;
		}
	}

	std::cout << std::endl << std::left << std::setw(25) << "keep-alive:" << 
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

bool	HTTPRequest::success() const {
	return _statusCode < 400;
}
