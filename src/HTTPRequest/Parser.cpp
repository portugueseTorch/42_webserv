#include "HTTPRequest/Parser.hpp"

HTTPParser::HTTPParser(std::list<Tok> lex) {
    _lex = lex;
    this->it = _lex.begin();
}

HTTPParser::~HTTPParser() {
    _lex.clear();
    _nodes.clear();
}

int HTTPParser::parse() {
    if (!validConfiguration()) {
        log(std::cerr, ERROR, "Syntax error in HTTP Request", "");
        return 1;
    }
	log(std::cout, SUCCESS, "HTTP Request successfully parsed", "");
    return 0;
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
	it++;
	return false;
}

bool HTTPParser::validEmptyLine() {
	if (it->type == Empty && it->content == "empty") {
		it++;
		return true;
	}
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

// <general_header> | <request_header> | <entity_header>
bool HTTPParser::validHeader() {
	std::list<Tok>::iterator tmp = it;
    int start_index = _nodes.size();
    if (it->type == End)
        return true;
	if (validGeneralHeader())
		return true;

	resetNodes(start_index);
	it = tmp;
	if (validRequestHeader())
		return true;
	resetNodes(start_index);
	it = tmp;
	if (validEntityHeader())
		return true;
	resetNodes(start_index);
	it = tmp;

	return false;
}

// <method> <request_URI> <http_version> 'CRLF'
bool HTTPParser::validRequestLine() {
	std::list<Tok>::iterator tmp = it;
	int start_index = _nodes.size();
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

bool HTTPParser::validMethod() {
	std::vector<std::string> validMethods = {
		"GET", "POST", "DELETE"};
	std::vector<std::string>::iterator methodIt = validMethods.begin(); 
	for (; methodIt != validMethods.end(); methodIt++) {
		if (*methodIt == it->content) {
			Node method_node(it->content, Method);
			_nodes.push_back(method_node);
			it++;
			return true;
		}
	}
	return false;
}

// '*' | <absolute_URI> | <absolute_path>
bool HTTPParser::validRequestURI() {
	if (it->content == "*" || it->content.find('/') == 0 || validAbsoluteURI()) {
		Node URI_node((it++)->content, URI);
		_nodes.push_back(URI_node);
		return true;
	}
	it++;
	return false;
}

bool HTTPParser::validAbsoluteURI() {
	std::transform(it->content.begin(), it->content.end(), it->content.begin(), ::tolower);
	std::size_t isHTTP = it->content.find("http://");
	std::cout << it->content.at(7) << std::endl;
	if (isHTTP == 0 && \
		it->content.find_last_of("http://") == 6 && \
		it->content.size() > 7 && \
		std::isalnum(it->content.at(7)))
		return true;
	return false;
}

bool HTTPParser::validHTTPVersion() {
	if ((it++)->content != ("HTTP/1.1"))
		return false;
	return true;
}

// [cache_control] [connection] [date] [pragma] [trailer] 
// [transfer_encoding] [upgrade] [via] [warning]
bool HTTPParser::validGeneralHeader() {
	return false;
}

// [accept] [accept_charset] [accept_encoding] [accept_language]
// [authorization] [expect] [from] [host] [if_match] [if_modified_since]
// [if_none_match] [if_range] [if_unmodified_since] [max_forwards]
// [proxy_authorization] [range] [referer] [TE] [user_agent]
bool HTTPParser::validRequestHeader() {
	bool nameSet = false;
	while (it != _lex.end() && it->type != Empty && it->type != End) {
		// while (it != _lex.end() && it->type == NL)
			// it++;
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
			// if (it != _lex.end())
			// 	it++;
			Node paramNode(fullText, Parameter);
			_nodes.push_back(paramNode);
			nameSet = false;
		} else if (it->type == NameTok && nameSet || it->type == ParamTok && !nameSet) {
			it++;
			return false;
		} else
			it++;
		std::cout << it->content << ", " << it->type << std::endl;
	}
	// it++;
	return true;
}

// [allow] [content_encoding] [content_language] [content_length]
// [content_location] [content_md5] [content_range] [content_type]
// [expires] [last_modified] [extension_header]
bool HTTPParser::validEntityHeader() {
	return false;
}

// 'CRLF' <message_body>
bool HTTPParser::validBody() {
	
	if (it == _lex.end() || it->type == End)
		return true;
	
			std::cout << "aqui" << std::endl;
	// if (validNL()) {
	// 	std::cout << it->content << ", " << it->type << std::endl;

	// 	return true;
	// }
	
	if (it != _lex.end() && !validEmptyLine())
		return false;
	it++;
	std::string fullText = "";
	while (it->type != End) {
		if (!fullText.empty())
			fullText.append(" ");
		fullText.append(it->content);
		it++;
	}
	Node bodyText(fullText, Body);
	_nodes.push_back(bodyText);
	
	return true;
}