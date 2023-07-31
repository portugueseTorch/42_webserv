#include "../inc/Parser.hpp"

Parser::~Parser() {
	std::vector<Token*>::iterator tmp = _tokens.begin();
	for (; tmp != _tokens.end(); tmp++)
		delete (*tmp);
	_tokens.clear();
}

void Parser::buildAST(std::vector<Tok*> lex) {
	_lex = lex;
	this->it = _lex.begin();
	if (validSimpleDir()) {
		std::cout << "\nSUCCESS: .conf file successfully parsed\n" << std::endl;
		displayAST();
	}
	else
		std::cerr << "ERROR: syntax error in .conf" << std::endl;
}

void Parser::displayAST() {
	std::cout << "##### PARSER OUTPUT #####" << std::endl;
	std::vector<Token*>::iterator tmp = _tokens.begin();
	for (; tmp != _tokens.end(); tmp++)
		(*tmp)->display();
}

//////////////////////////////////////////////////

bool Parser::validConfiguration() {
	std::vector<Tok*>::iterator tmp = it;
	std::vector<Token*> backup = _tokens;
	if ((*it)->type == TokenType::End)
		return false;

	std::cout << "\nTesting validConfiguration at: " << (*it)->content << std::endl;
	if (conf_case1())
		return true;
	log("\t!!! NOT CONF_CASE1");
	_tokens.clear();
	_tokens = backup;
	it = tmp;
	if (conf_case2())
		return true;
	log("\t!!! NOT CONF_CASE2");
	_tokens.clear();
	_tokens = backup;
	it = tmp;

	return false;
}

// <server_block> <configuration>
bool Parser::conf_case1() {
	std::cout << "\tEntered conf_case1 with: " << (*it)->content << std::endl;
	if (!validServerBlock())
		return false;
	if (validConfiguration())
		return true;
	return false;
}

// <server_block>
bool Parser::conf_case2() {
	std::cout << "\tEntered conf_case2 with: " << (*it)->content << std::endl;
	return validServerBlock();
}

/***************************************/

bool Parser::validServerBlock() {
	std::vector<Tok*>::iterator tmp = it;
	std::vector<Token*> backup = _tokens;
	if ((*it)->type == TokenType::End)
		return false;

	std::cout << "\nTesting validServerBlock at: " << (*it)->content << std::endl;
	if (server_block_case1())
		return true;
	_tokens.clear();
	_tokens = backup;
	it = tmp;
	log("\t!!! NOT SERVER_BLOCK_CASE1");
	return false;
}

// [server] '{' <directives> '}'
bool Parser::server_block_case1() {
	std::cout << "\tEntered server_block_case1 with: " << (*it)->content << std::endl;

	if ((*it)->content != "server")
		return false;
	_tokens.push_back(new ServerToken);
	it++;
	if ((*it)->type != TokenType::OpenBrack)
		return false;
	it++;
	if (!validDirectives())
		return false;
	std::cout << "\tRe-entered server_block_case1 at: " << (*it)->content << std::endl;
	if ((*it)->type != TokenType::CloseBrack)
		return false;
	it++;
	std::cout << "\t\tSuccess: [server] '{' <directives> '}'" << std::endl;
	return true;
}

/***************************************/

bool Parser::validDirectives() {
	std::vector<Tok*>::iterator tmp = it;
	std::vector<Token*> backup = _tokens;
	if ((*it)->type == TokenType::End)
		return false;

	std::cout << "\nTesting validDirectives at: " << (*it)->content << std::endl;
	if (dir_case1())
		return true;
	log("\t!!! NOT DIR_CASE1");
	_tokens.clear();
	_tokens = backup;
	it = tmp;
	if (dir_case2())
		return true;
	log("\t!!! NOT DIR_CASE2");
	_tokens.clear();
	_tokens = backup;
	it = tmp;
	return false;
}

// <block_dir> <directives>
bool Parser::dir_case1() {
	std::cout << "\tEntered dir_case1 with: " << (*it)->content << std::endl;

	if (!validBlockDir())
		return false;
	std::cout << "\tRe-entered dir_case1 at: " << (*it)->content << std::endl;
	if (it == _lex.end())
		return false;
	if (!validBlockDir())
		return false;
	std::cout << "\tRe-entered dir_case1 at: " << (*it)->content << std::endl;
	return true;
}

// <simple_dir> <directives>
bool Parser::dir_case2() {
	std::cout << "\tEntered dir_case2 with: " << (*it)->content << std::endl;

	if (!validSimpleDir())
		return false;
	std::cout << "\tRe-entered dir_case2 at: " << (*it)->content << std::endl;
	if (it == _lex.end())
		return false;
	if (!validBlockDir())
		return false;
	std::cout << "\tRe-entered dir_case2 at: " << (*it)->content << std::endl;
	return true;
}

/***************************************/

bool Parser::validBlockDir() {
	std::vector<Tok*>::iterator tmp = it;
	std::vector<Token*> backup = _tokens;
	if ((*it)->type == TokenType::End)
		return false;

	std::cout << "\nTesting validBlockDir at: " << (*it)->content << std::endl;
	if (block_dir_case1())
		return true;
	log("\t!!! NOT BLOCK_DIR_CASE1");
	_tokens.clear();
	_tokens = backup;
	it = tmp;

	return false;
}

// [Name] <simple_dir> '{' <simple_dir> '}'
bool Parser::block_dir_case1() {
	std::cout << "\tEntered block_dir_case1 with: " << (*it)->content << std::endl;

	if ((*it)->type != TokenType::Name)
		return false;
	_tokens.push_back(new NameToken((*it)->content));
	it++;

	if (!validSimpleDir())
		return false;
	std::cout << "\tRe-entered block_dir_case1 with: " << (*it)->content << std::endl;
	if ((*it)->type != TokenType::OpenBrack)
		return false;
	it++;
	if (!validSimpleDir())
		return false;
	std::cout << "\tRe-entered block_dir_case1 with: " << (*it)->content << std::endl;
	if ((*it)->type != TokenType::CloseBrack)
		return false;
	it++;
	std::cout << "\t\tSuccess: [Name] <param_list> '{' <simple_dir> '}'" << std::endl;
	return true;
}

/***************************************/

bool Parser::validSimpleDir() {
	std::vector<Tok*>::iterator tmp = it;
	std::vector<Token*> backup = _tokens;
	if ((*it)->type == TokenType::End)
		return false;

	std::cout << "\nTesting validSimpleDir at: " << (*it)->content << std::endl;
	if (simple_dir_case1())
		return true;
	log("\t!!! NOT SIMPLE_DIR_CASE1");
	_tokens.clear();
	_tokens = backup;
	it = tmp;
	if (simple_dir_case2())
		return true;
	log("\t!!! NOT SIMPLE_DIR_CASE2");
	_tokens.clear();
	_tokens = backup;
	it = tmp;
	if (simple_dir_case3())
		return true;
	log("\t!!! NOT SIMPLE_DIR_CASE3");
	_tokens.clear();
	_tokens = backup;
	it = tmp;
	return false;
}

// [Name] <simple_dir>
bool Parser::simple_dir_case1() {
	std::cout << "\tEntered simple_dir_case1 with: " << (*it)->content << std::endl;

	if (name_flag)
		return false;
	if ((*it)->type != TokenType::Name && (*it)->type != TokenType::Parameter)
		return false;
	_tokens.push_back(new NameToken((*it)->content));
	name_flag = true;
	it++;
	if (!validSimpleDir())
		return false;
	std::cout << "\t\tSuccess: [Name] <param_lst>" << std::endl;
	return true;
}

// [Parameter] <simple_dir>
bool Parser::simple_dir_case2() {
	std::cout << "\tEntered simple_dir_case1 with: " << (*it)->content << std::endl;

	if ((*it)->type != TokenType::Parameter)
		return false;
	_tokens.push_back(new ParameterToken((*it)->content));
	it++;
	if (!validSimpleDir())
		return false;
	std::cout << "\t\tSuccess: [Parameter] <simple_dir>" << std::endl;
	return true;
}

// [Parameter] ';'
bool Parser::simple_dir_case3() {
	std::cout << "\tEntered simple_dir_case1 with: " << (*it)->content << std::endl;

	if ((*it)->type != TokenType::Parameter)
		return false;
	_tokens.push_back(new ParameterToken((*it)->content));
	it++;
	if ((*it)->type != TokenType::Semicolon)
		return false;
	std::cout << "\t\tSuccess: [Parameter] <simple_dir>" << std::endl;
	return true;
}
