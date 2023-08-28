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

// void HTTPParser::displayAST() {
// 	std::cout << "\n##### PARSER OUTPUT #####" << std::endl;
// 	std::cout << "\n";
// 	std::list<Node>::iterator tmp = _nodes.begin();
// 	for (; tmp != _nodes.end(); tmp++)
// 		tmp->display();
// 	std::cout << std::endl;
// }

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

bool HTTPParser::validConfiguration() {
    std::list<Tok>::iterator tmp = it;
    int start_index = _nodes.size();
    if (it->type == End)
        return true;
    if (conf_case1())
        return true;
    resetNodes(start_index);
    it = tmp;
    if (conf_case2())
        return true;
    resetNodes(start_index);
    it = tmp;
    if (conf_case3())
        return true;
    resetNodes(start_index);
    it = tmp;

    return false;
}

// <request_line> <general_header> 'CRLF' [ 'CRLF' message_body]
bool HTTPParser::conf_case1() {
    return false;
}

// <request_line> <request_header> 'CRLF' [ 'CRLF' message_body]
bool HTTPParser::conf_case2() {
    return false;
}

// <request_line> <entity_header> 'CRLF' [ 'CRLF' message_body]
bool HTTPParser::conf_case3() {
    return false;
}