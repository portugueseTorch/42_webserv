#include "../inc/Webserv.hpp"

Node::Node(std::string content, NodeType type) : _content(content), _type(type) {}

Node::~Node() {}

void Node::display() {
	std::cout << "[ \"" << _content << "\", " << _type << " ]" << std::endl;
}

//////////////////////////////////////////////////

Parser::Parser(std::list<Tok> lex) {
	_lex = lex;
	this->it = _lex.begin();
}

Parser::~Parser() {
	_lex.clear();
	_nodes.clear();
}

bool Parser::validSemicolon() {
	for (; it != _lex.end(); it++)
	{
		if (it->type == TokenType::NL)
		{
			std::list<Tok>::iterator i = it;
			if (i != _lex.begin())
			{
				i--;
				if (i->type != TokenType::SemiTok && i->type != TokenType::OpenBrack && i->type != TokenType::CloseBrack)
					return false;
			}
			_lex.erase(it++);
			it--;
		}
	}
	return true;
}

int Parser::buildAST() {
	if (!validSemicolon()) {
		log(std::cerr, MsgType::ERROR, "Syntax error in .conf", "");
		return 1;
	}
	// std::list<Tok>::iterator it = _lex.begin();
	// for (; it != _lex.end(); it++)
	// 	std::cout << "[ \"" << it->content << "\", " << it->type << " ]" << std::endl;
	this->it = _lex.begin();
	if (!validConfiguration()) {
		log(std::cerr, MsgType::ERROR, "Syntax error in .conf", "");
		return 1;
	}
	log(std::cout, MsgType::SUCCESS, ".conf file successfully parsed", "");
	return 0;
}

void Parser::displayAST() {
	std::cout << "\n##### PARSER OUTPUT #####" << std::endl;
	std::cout << "\n";
	std::list<Node>::iterator tmp = _nodes.begin();
	for (; tmp != _nodes.end(); tmp++)
		tmp->display();
	std::cout << std::endl;
}

void Parser::resetNodes(int start) {
	std::list<Node>::iterator i = _nodes.begin();
	for (; start > 0 && i != _nodes.end(); start--)
		i++;
	for (; i != _nodes.end(); i++)
	{
		_nodes.erase(i++);
		i--;
	}
}

const std::list<Node> &Parser::getNodes() const {
	return _nodes;
}

//////////////////////////////////////////////////

bool Parser::validConfiguration() {
	std::list<Tok>::iterator tmp = it;
	int start_index = _nodes.size();
	if (it->type == TokenType::End)
		return true;

	if (conf_case1())
		return true;
	resetNodes(start_index);
	it = tmp;

	return false;
}

// <server_block> <configuration>
bool Parser::conf_case1() {
	if (!validServerBlock())
		return false;
	if (!validConfiguration())
		return false;
	return true;
}

/***************************************/

bool Parser::validServerBlock() {
	std::list<Tok>::iterator tmp = it;
	int start_index = _nodes.size();
	if (it->type == TokenType::End)
		return false;

	if (server_block_case1())
		return true;
	resetNodes(start_index);
	it = tmp;

	return false;
}

// [server] <block>
bool Parser::server_block_case1() {

	if (it->content != "server")
		return false;
	Node server_node("server", NodeType::ServerBlock);
	_nodes.push_back(server_node);
	it++;
	if (!validBlock())
		return false;
	return true;
}

/***************************************/

bool Parser::validBlock() {
	std::list<Tok>::iterator tmp = it;
	int start_index = _nodes.size();
	if (it->type == TokenType::End)
		return false;

	if (block_case1())
		return true;
	resetNodes(start_index);
	it = tmp;

	return false;
}

// '{' <directives> '}'
bool Parser::block_case1() {

	if (it->type != TokenType::OpenBrack)
		return false;
	Node open_brack_node("{", NodeType::OpenBracket);
	_nodes.push_back(open_brack_node);
	it++;
	if (!validDirectives())
		return false;
	if (it->type != TokenType::CloseBrack)
		return false;
	Node close_brack_node("}", NodeType::CloseBracket);
	_nodes.push_back(close_brack_node);
	it++;
	return true;
}

/***************************************/

bool Parser::validDirectives() {
	std::list<Tok>::iterator tmp = it;
	int start_index = _nodes.size();
	if (it->type == TokenType::End)
		return false;

	if (directives_case1())
		return true;
	resetNodes(start_index);
	it = tmp;
	if (directives_case2())
		return true;
	resetNodes(start_index);
	it = tmp;
	if (directives_case3())
		return true;
	resetNodes(start_index);
	it = tmp;
	if (directives_case4())
		return true;
	resetNodes(start_index);
	it = tmp;

	return false;
}

// <block_dirs> <directives>
bool Parser::directives_case1() {

	if (!validBlockDirectives())
		return false;
	if (!validDirectives())
		return false;
	return true;
}

// <simple_dir_lst> <directives>
bool Parser::directives_case2() {

	if (!validSimpleDirectiveList())
		return false;
	if (!validDirectives())
		return false;
	return true;
}

// <block_dirs>
bool Parser::directives_case3() {

	if (!validBlockDirectives())
		return false;
	return true;
}

// <simple_dir_lst>
bool Parser::directives_case4() {

	if (!validSimpleDirectiveList())
		return false;
	return true;
}

/***************************************/

bool Parser::validBlockDirectives() {
	std::list<Tok>::iterator tmp = it;
	int start_index = _nodes.size();
	if (it->type == TokenType::End)
		return false;

	if (block_directives_case1())
		return true;
	resetNodes(start_index);
	it = tmp;
	if (block_directives_case2())
		return true;
	resetNodes(start_index);
	it = tmp;

	return false;
}

// <simple_block> <block_dirs>
bool Parser::block_directives_case1() {

	if (!validSimpleBlock())
		return false;
	if (!validBlockDirectives())
		return false;
	return true;
}

// <simple_block>
bool Parser::block_directives_case2() {

	if (!validSimpleBlock())
		return false;
	return true;
}

/***************************************/

bool Parser::validSimpleBlock() {
	std::list<Tok>::iterator tmp = it;
	int start_index = _nodes.size();
	if (it->type == TokenType::End)
		return false;

	if (simple_block_case1())
		return true;
	resetNodes(start_index);
	it = tmp;

	return false;
}

// [name] [parameter] '{' <simple_dir_lst> '}'
bool Parser::simple_block_case1() {

	if (it->type != TokenType::NameTok && it->type != TokenType::ParamTok)
		return false;
	std::string n = it->content;
	it++;
	if (it->type != TokenType::NameTok && it->type != TokenType::ParamTok)
		return false;
	std::string content = n + " " + it->content;
	it++;
	Node pre_block_node(content, NodeType::LocationBlock);
	_nodes.push_back(pre_block_node);
	if (it->type != TokenType::OpenBrack)
		return false;
	Node open_brack_node("{", NodeType::OpenBracket);
	_nodes.push_back(open_brack_node);
	it++;
	if (!validSimpleDirectiveList())
		return false;
	if (it->type != TokenType::CloseBrack)
		return false;
	Node close_brack_node("}", NodeType::CloseBracket);
	_nodes.push_back(close_brack_node);
	it++;
	return true;
}

/***************************************/

bool Parser::validSimpleDirectiveList()
{
	std::list<Tok>::iterator tmp = it;
	int start_index = _nodes.size();
	if (it->type == TokenType::End)
		return false;

	if (simple_directive_list_case1())
		return true;
	resetNodes(start_index);
	it = tmp;
	if (simple_directive_list_case2())
		return true;
	resetNodes(start_index);
	it = tmp;

	return false;
}

// <simple_dir> <simple_dir_lst>
bool Parser::simple_directive_list_case1()
{

	if (!validSimpleDirective())
		return false;
	if (!validSimpleDirectiveList())
		return false;
	return true;
}

// <simple_dir>
bool Parser::simple_directive_list_case2()
{

	if (!validSimpleDirective())
		return false;
	return true;
}

/***************************************/

bool Parser::validSimpleDirective()
{
	std::list<Tok>::iterator tmp = it;
	int start_index = _nodes.size();
	if (it->type == TokenType::End)
		return false;

	if (simple_directive_case1())
		return true;
	resetNodes(start_index);
	it = tmp;

	return false;
}

// [name] <parameter_lst>
bool Parser::simple_directive_case1()
{

	if (it->type != TokenType::NameTok && it->type != TokenType::ParamTok)
		return false;
	Node name_node(it->content, NodeType::Name);
	_nodes.push_back(name_node);
	it++;

	if (!validParameterList())
		return false;
	return true;
}

/***************************************/

bool Parser::validParameterList()
{
	std::list<Tok>::iterator tmp = it;
	int start_index = _nodes.size();
	if (it->type == TokenType::End)
		return false;

	if (parameter_list_case1())
		return true;
	resetNodes(start_index);
	it = tmp;
	if (parameter_list_case2())
		return true;
	resetNodes(start_index);
	it = tmp;

	return false;
}

// [parameter] ';'
bool Parser::parameter_list_case1()
{

	if (it->type != TokenType::ParamTok)
		return false;
	Node parameter_node(it->content, NodeType::Parameter);
	_nodes.push_back(parameter_node);
	it++;
	if (it->type != TokenType::SemiTok)
		return false;
	it++;
	return true;
}

// [parameter] <parameter_lst>
bool Parser::parameter_list_case2()
{

	if (it->type != TokenType::ParamTok)
		return false;
	Node parameter_node(it->content, NodeType::Parameter);
	_nodes.push_back(parameter_node);
	it++;
	if (!validParameterList())
		return false;
	return true;
}
