#include "HTTPRequest/HTTPRequest.hpp"

enum possibleLineTypes {
	REQUEST_LINE,
	CONTENT_LENGTH,
	CONNECTION,
	GENERAL_HEADER
};

HTTPRequest::HTTPRequest():
	isCGI(false),
	fullyParsed(false),
	_statusCode(200),
	_keepAlive(true),
	_contentLength(0),
	_emptyLine(false) {
	_port = htons(8080);
	_ip_address = inet_addr("0.0.0.0");
}

HTTPRequest::~HTTPRequest() {};

void HTTPRequest::process(std::string request) {
	std::string headerLine;
	
	_content += request;

	size_t newLine = _content.find("\r\n");
	while (newLine != std::string::npos) {
		if (!_emptyLine) {
			headerLine = _content.substr(0, newLine);
			if (headerLine.empty())
				_emptyLine = true;
			else
				processHeaderLine(headerLine);
		} else {
			if (_contentLength == 0 && (_content.size() || _body.size())) {
				_statusCode = 411;
			} else {
				_body += _content;
			}
		}
		_content = _content.substr(newLine + 2);
		newLine = _content.find("\r\n");
	}

	if (_emptyLine) {
		if (_contentLength == 0 && (_content.size() || _body.size())) {
			_statusCode = 411;
		} else {
			_body += _content;
		}
		_content = "";
		if (!_contentLength && _body.empty()) {
			fullyParsed = true;
		} else if (_body.length() >= _contentLength) {
			_body = _body.length() > _contentLength ? \
					_body.substr(0, _contentLength): \
					_body;
			fullyParsed = true;
		}
	}
}

void	HTTPRequest::setPort(uint32_t port) {
	_port = port;
}

void	HTTPRequest::setIPAddress(in_addr_t ip_address) {
	_ip_address = ip_address;
}

void HTTPRequest::setURI(std::string uri) {
	_requestURI = uri;
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

bool	HTTPRequest::processHeaderLine(std::string headerLine) {
	bool validLine = true;
	int lineType = getLineType(headerLine);

	switch(lineType) {
		case REQUEST_LINE:
			validLine = checkRequestLine(headerLine);
			break;
		case CONTENT_LENGTH:
			validLine = checkContentLength(headerLine.substr(headerLine.find(":") + 1));
			break;
		case CONNECTION:
			validLine = checkConnection(headerLine.substr(headerLine.find(":") + 1));
			break;
		default:
			validLine = addParam(headerLine);
	}

	return validLine;
}

bool	HTTPRequest::checkRequestLine(std::string & headerLine) {
	if (!validMethod(headerLine))
		return false;
	if (!validRequestURI(headerLine))
		return false;
	if (!validHTTPVersion(headerLine))
		return false;
	return true;
}

bool HTTPRequest::validMethod(std::string & headerLine) {
	if (alreadyExists(_method))
		return false;

	if (headerLine.find("GET") == 0 || \
		headerLine.find("POST") == 0 || \
		headerLine.find("DELETE") == 0) {
		_method = headerLine.substr(0, headerLine.find(" "));
		headerLine.erase(0, headerLine.find(" ") + 1);
		return true;
	}
	_statusCode = 400;
	return false;
}

bool HTTPRequest::validRequestURI(std::string & headerLine) {
	std::stringstream	ss(headerLine);
	std::string			word;

	if (alreadyExists(_requestURI))
		return false;

	ss >> word;
	if (word == "*" || word.find('/') == 0 || validAbsoluteURI(word)) {
		cleanUpURI(word);
		if (!success())
			return false;
		headerLine.erase(0, headerLine.find(" ") + 1);
		return true;
	}
	_statusCode = 400;
	return false;
}

void HTTPRequest::cleanUpURI(std::string req) {
	std::string u;
	if (req != "/" && req[req.length() - 1] == '/')
		u = req.substr(0, req.length() - 1);
	else
		u = req;
	if (u.find("cgi-bin") != std::string::npos && \
		(u.find(".py") != std::string::npos || \
		u == "/cgi-bin" || u == "/cgi-bin/")) {
		isCGI = true;
		_requestURI = u.substr(0, u.find('?'));
		extractQuery(u);
	} else {
		_requestURI = u;
	}
}

bool HTTPRequest::validAbsoluteURI(std::string req) {
	std::transform(req.begin(), req.end(), req.begin(), ::tolower);
	std::size_t isHTTP = req.find("http://");
	if (isHTTP == 0 && \
		req.find_last_of("http://") == 6 && \
		req.size() > 7 && \
		std::isalnum(req.at(7)))
		return true;
	return false;
}

bool HTTPRequest::validHTTPVersion(std::string & headerLine) {
	std::string toKeep(headerLine);

	if (alreadyExists(_protocol))
		return false;

	size_t isHTTP = headerLine.find("HTTP/");
	if (isHTTP == 0) {
		headerLine.erase(0, 5);
		double version = std::atof(headerLine.c_str());
		if (version == 1 || version == 1.1 || version == 2 || version == 3) {
			_protocol = toKeep;
			return true;
		}
	}
	_statusCode = 400;
	return false;
}

bool HTTPRequest::checkContentLength(std::string headerLine) {
	if (headerLine.find_first_not_of("0123456789 \t\v\f") != std::string::npos) {
		_statusCode = 400;
		return false;
	}
	_contentLength = std::atoi(headerLine.c_str());
	return true;
}

bool HTTPRequest::checkConnection(std::string headerLine) {
	headerLine = headerLine.substr(headerLine.find_first_not_of(" \t\v\f") != std::string::npos);
	if (headerLine != "keep-alive" && headerLine != "closed") {
		_statusCode = 400;
		return false;
	}
	_keepAlive = (headerLine == "keep-alive") ? true : false;
	return true;
}

bool HTTPRequest::addParam(std::string headerLine) {
	std::string param;
	std::string value;
	size_t		sep;

	sep = headerLine.find(":");
	if (sep == std::string::npos) {
		_statusCode = 400;
		return false;
	}

	param = headerLine.substr(0, sep);
	std::transform(param.begin(), param.end(), param.begin(), ::tolower);

	value = headerLine.substr(sep + 2);
	
	if (param.size() && value.size()) {
		_params[param] = value;
		return true;
	}
	_statusCode = 400;
	return false;
}

int	HTTPRequest::getLineType(std::string headerLine) {
	if (headerLine.find("GET") == 0 || \
		headerLine.find("POST") == 0 || \
		headerLine.find("DELETE") == 0)
		return REQUEST_LINE;
	if (headerLine.find("Content-Length:") == 0)
		return CONTENT_LENGTH;
	if (headerLine.find("Connection:") == 0)
		return CONNECTION;
	return GENERAL_HEADER;
}

void	HTTPRequest::extractQuery(std::string URI) {
	std::string query = URI.substr(URI.find('?') + 1, URI.size());
	std::stringstream squery(query);
	std::string buf;
	std::string param;
	std::string value;

	if (URI.find("?") == std::string::npos)
		return ;
	while (getline(squery, buf, '&')) {
		size_t sep = buf.find('=');
		if (sep == std::string::npos) {
			log(std::cerr, ERROR, "Invalid proxy query", buf);
			_statusCode = 400;
		}
		param = buf.substr(0, sep);
		value = buf.substr(sep + 1, buf.size());
		if (param.size() && value.size())
			_query.push_back(buf);
		else {
			log(std::cerr, ERROR, "Invalid proxy query", buf);
			_statusCode = 400;
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
	std::vector<std::string>::iterator queryIt;

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

	queryIt = _query.begin();
	for (; queryIt != _query.end(); queryIt++) {
		std::cout << *queryIt << std::endl;
	}

	if (_body.size()) {
		std::cout << "\r\n" << _body << std::endl;
	}
}

bool	HTTPRequest::success() const {
	return _statusCode < 400;
}

bool	HTTPRequest::alreadyExists(std::string param) {
	if (param.size()) {
		_statusCode = 400;
		return true;
	}
	return false;
}