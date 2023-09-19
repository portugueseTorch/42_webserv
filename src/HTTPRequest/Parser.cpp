#include "HTTPRequest/Parser.hpp"

HTTPParser::HTTPParser(std::list<Tok> lex) {
	_errReason = "";
    _lex = lex;
    this->it = _lex.begin();
}

HTTPParser::~HTTPParser() {
	if (_lex.size())
    	_lex.clear();
    _nodes.clear();
}

void HTTPParser::parse() {
    if (!validConfiguration()) {
		throw invalidSyntaxException(_errReason.c_str());
		return ;
    }
	log(std::cout, SUCCESS, "HTTP Request successfully parsed", "");
}

void HTTPParser::displayAST() {
	std::cout << "\n##### PARSER OUTPUT #####" << std::endl;
	std::cout << "\n";
	std::list<Node>::iterator tmp = _nodes.begin();
	for (; tmp != _nodes.end(); tmp++)
		tmp->display();
	std::cout << std::endl;
}

void HTTPParser::resetNodes(int start) {
	std::list<Node>::iterator i = _nodes.begin();
	for (; start > 0 && i != _nodes.end(); start--)
		i++;
	for (; i != _nodes.end(); i++)
	{
		_nodes.erase(i++);
		i--;
	}
}

const std::list<Node> &HTTPParser::getNodes() const {
	return _nodes;
}

bool HTTPParser::validNL() {
	if (it->type == NL && it->content == "nl") {
		it++;
		return true;
	}
	_errReason = "missing new line";
	it++;
	return false;
}

bool HTTPParser::validEmptyLine() {
	if (it->type == Empty && it->content == "empty") {
		it++;
		return true;
	}
	_errReason = "missing empty line";
	it++;
	return false;
}

// <request_line> <header> [request_body]
bool HTTPParser::validConfiguration() {
    if (it->type == End)
        return true;

    if (!validRequestLine())
		return false;
	if (!validHeader())
		return false;
	if (!validBody())
		return false;
    return true;
}

// <method> <request_URI> <http_version> 'CRLF'
bool HTTPParser::validRequestLine() {
	if (it->type == End)
		return false;
	
	if (!validMethod())
		return false;
	if (!validRequestURI())
		return false;
	if (!validHTTPVersion())
		return false;
	if (!validNL())
		return false;
	return true;
}

// <general_header> | <request_header> | <entity_header>
bool HTTPParser::validHeader() {
	std::list<Tok>::iterator tmp = it;
    int start_index = _nodes.size();
    if (it->type == End)
		return true;
	
	if (validRequestHeader())
		return true;
	resetNodes(start_index);
	it = tmp;

	return false;
}


bool HTTPParser::validMethod() {
	std::string	methods[] = {"GET", "POST", "DELETE", "OPTIONS"}; 
	std::vector<std::string> validMethods(methods, methods + sizeof(methods) / sizeof(std::string));

	std::vector<std::string>::iterator methodIt = validMethods.begin(); 
	for (; methodIt != validMethods.end(); methodIt++) {
		if (*methodIt == it->content) {
			Node method_node(it->content, Method);
			_nodes.push_back(method_node);
			it++;
			return true;
		}
	}
	_errReason = "wrong HTTP request method";
	return false;
}

// '*' | <absolute_URI> | <absolute_path>
bool HTTPParser::validRequestURI() {
	if (it->content == "*" || it->content.find('/') == 0 || validAbsoluteURI()) {
		Node URI_node((it++)->content, URI);
		_nodes.push_back(URI_node);
		return true;
	}
	_errReason = "wrong HTTP request URI " + it->content;
	it++;
	return false;
}

bool HTTPParser::validAbsoluteURI() {
	std::transform(it->content.begin(), it->content.end(), it->content.begin(), ::tolower);
	std::size_t isHTTP = it->content.find("http://");
	if (isHTTP == 0 && \
		it->content.find_last_of("http://") == 6 && \
		it->content.size() > 7 && \
		std::isalnum(it->content.at(7)))
		return true;
	return false;
}

bool HTTPParser::validHTTPVersion() {
	std::string temp = (it++)->content;
	std::string toKeep(temp);
	size_t isHTTP = temp.find("HTTP/");
	if (isHTTP == 0) {
		temp.erase(0, 5);
		double version = std::atof(temp.c_str());
		if (version == 1 || version == 1.1 || version == 2 || version == 3) {
			Node HTTPVersion(toKeep, Protocol);
			_nodes.push_back(HTTPVersion);
			return true;
		}
	}
	_errReason = "wrong HTTP protocol" + toKeep;
	return false;
}

// [accept] [accept_charset] [accept_encoding] [accept_language]
// [authorization] [expect] [from] [host] [if_match] [if_modified_since]
// [if_none_match] [if_range] [if_unmodified_since] [max_forwards]
// [proxy_authorization] [range] [referer] [TE] [user_agent]
bool HTTPParser::validRequestHeader() {
	bool nameSet = false;
	while (it != _lex.end() && it->type != Empty && it->type != End) {
		if (!nameSet && it->type == NameTok) {
			Node nameNode(it->content, Name);
			_nodes.push_back(nameNode);
			nameSet = true;
			it++;
		} else if (nameSet && it->type == ParamTok) {
			std::string fullText = "";
			while (it != _lex.end() && it->type == ParamTok) {
				if (!fullText.empty())
					fullText.append(" ");
				fullText.append(it->content);
				it++;
			}
			Node paramNode(fullText, Parameter);
			_nodes.push_back(paramNode);
			nameSet = false;
		} else if ((it->type == NameTok && nameSet) || (it->type == ParamTok && !nameSet)) {
			if (nameSet)
				_errReason = "wrong HTTP request header " + ((--it)++)->content;
			else
				_errReason = "wrong HTTP request param " + it->content;
			it++;
			return false;
		} else
			it++;
	}
	return true;
}

// 'CRLF' <message_body>
bool HTTPParser::validBody() {
	std::string body;
	bool nextSpace = false;

	if (it != _lex.end() && validEmptyLine() && validNL()) {
		while (it != _lex.end() && it->type != End) {
			if (it->type == Empty) {
				body.append("\r");
				nextSpace = false;
			}
			else if (it->type == NL) {
				body.append("\n");
				nextSpace = false;
			}
			else {
				if (nextSpace) {
					body.append(" ");
				}
				body.append(it->content);
				nextSpace = true;
			}
			it++;
		}
		if (body.size()) {
			Node bodyText(body, Body);
			_nodes.push_back(bodyText);
			return true;
		}
	} else if (it == _lex.end() || it->type == End)
		return true;
	//nao esquecer de conferir
	_errReason = "empty HTTP request body";
	return false;
}