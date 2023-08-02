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
	if (validConfiguration()) {
		displayAST();
		std::cout << "\nSUCCESS: .conf file successfully parsed\n" << std::endl;
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

void Parser::parseAST() {
	return ;
}

void Parser::resetTokens(std::vector<Token*>::iterator start) {
	if (_tokens.empty() || start == _tokens.end())
		return ;
	start++;
	for (; start != _tokens.end(); start++)
		delete (*start);
	_tokens.clear();
}

//////////////////////////////////////////////////

bool Parser::validConfiguration() {
	std::vector<Tok*>::iterator tmp = it;
	std::vector<Token*> backup = _tokens;
	if ((*it)->type == TokenType::End)
		return true;

	std::cout << "\nTesting validConfiguration at: " << (*it)->content << std::endl;
	if (conf_case1())
		return true;
	log("\t!!! NOT CONF_CASE1");
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
	std::cout << "\t\tSUCCESSFULLY EXITED FIRST SERVER BLOCK with: " << (*it)->content << "\n\n" << std::endl;
	if (!validConfiguration())
		return false;
	return true;
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
	log("\t!!! NOT SERVER_BLOCK_CASE1");
	_tokens.clear();
	_tokens = backup;
	it = tmp;

	return false;
}

// [server] <block>
bool Parser::server_block_case1() {
	std::cout << "\tEntered server_block_case1 with: " << (*it)->content << std::endl;

	if ((*it)->content != "server")
		return false;
	_tokens.push_back(new ServerToken);
	it++;
	if (!validBlock())
		return false;
	std::cout << "\tRe-entered server_block_case1 at: " << (*it)->content << std::endl;
	return true;
}

/***************************************/

bool Parser::validBlock() {
	std::vector<Tok*>::iterator tmp = it;
	std::vector<Token*> backup = _tokens;
	if ((*it)->type == TokenType::End)
		return false;

	std::cout << "\nTesting validBlock at: " << (*it)->content << std::endl;
	if (block_case1())
		return true;
	log("\t!!! NOT BLOCK_CASE1");
	_tokens.clear();
	_tokens = backup;
	it = tmp;

	return false;
}

// '{' <directives> '}'
bool Parser::block_case1() {
	std::cout << "\tEntered block_case1 with: " << (*it)->content << std::endl;

	if ((*it)->type != TokenType::OpenBrack)
		return false;
	_tokens.push_back(new OpenBracketToken);
	it++;
	if (!validDirectives())
		return false;
	std::cout << "\tRe-entered block_case1 at: " << (*it)->content << std::endl;
	if ((*it)->type != TokenType::CloseBrack)
		return false;
	_tokens.push_back(new CloseBracketToken);
	it++;
	return true;
}

/***************************************/

bool Parser::validDirectives() {
	std::vector<Tok*>::iterator tmp = it;
	std::vector<Token*> backup = _tokens;
	if ((*it)->type == TokenType::End)
		return false;

	std::cout << "\nTesting validDirectives at: " << (*it)->content << std::endl;
	if (directives_case1())
		return true;
	log("\t!!! NOT DIRECTIVES_CASE1");
	_tokens.clear();
	_tokens = backup;
	it = tmp;
	if (directives_case2())
		return true;
	log("\t!!! NOT DIRECTIVES_CASE2");
	_tokens.clear();
	_tokens = backup;
	it = tmp;
	if (directives_case3())
		return true;
	log("\t!!! NOT DIRECTIVES_CASE3");
	_tokens.clear();
	_tokens = backup;
	it = tmp;
	if (directives_case4())
		return true;
	log("\t!!! NOT DIRECTIVES_CASE4");
	_tokens.clear();
	_tokens = backup;
	it = tmp;

	return false;
}

// <block_dirs> <directives>
bool Parser::directives_case1() {
	std::cout << "\tEntered directives_case1 with: " << (*it)->content << std::endl;

	if (!validBlockDirectives())
		return false;
	std::cout << "\tRe-entered directives_case1 with: " << (*it)->content << std::endl;
	if (!validDirectives())
		return false;
	return true;
}

// <simple_dir_lst> <directives>
bool Parser::directives_case2() {
	std::cout << "\tEntered directives_case2 with: " << (*it)->content << std::endl;

	if (!validSimpleDirectiveList())
		return false;
	std::cout << "\tRe-entered directives_case2 with: " << (*it)->content << std::endl;
	if (!validDirectives())
		return false;
	return true;
}

// <block_dirs>
bool Parser::directives_case3() {
	std::cout << "\tEntered directives_case3 with: " << (*it)->content << std::endl;

	if (!validBlockDirectives())
		return false;
	return true;
}

// <simple_dir_lst>
bool Parser::directives_case4() {
	std::cout << "\tEntered directives_case4 with: " << (*it)->content << std::endl;

	if (!validSimpleDirectiveList())
		return false;
	return true;
}

/***************************************/

bool Parser::validBlockDirectives() {
	std::vector<Tok*>::iterator tmp = it;
	std::vector<Token*> backup = _tokens;
	if ((*it)->type == TokenType::End)
		return false;

	std::cout << "\nTesting validBlockDirectives at: " << (*it)->content << std::endl;
	if (block_directives_case1())
		return true;
	log("\t!!! NOT BLOCK_DIRECTIVES_CASE1");
	_tokens.clear();
	_tokens = backup;
	it = tmp;
	if (block_directives_case2())
		return true;
	log("\t!!! NOT BLOCK_DIRECTIVES_CASE2");
	_tokens.clear();
	_tokens = backup;
	it = tmp;

	return false;
}

// <simple_block> <block_dirs>
bool Parser::block_directives_case1() {
	std::cout << "\tEntered block_directives_case1 with: " << (*it)->content << std::endl;

	if (!validSimpleBlock())
		return false;
	std::cout << "\tRe-entered block_directives_case1 with: " << (*it)->content << std::endl;
	if (!validBlockDirectives())
		return false;
	return true;
}

// <simple_block>
bool Parser::block_directives_case2() {
	std::cout << "\tEntered block_directives_case2 with: " << (*it)->content << std::endl;

	if (!validSimpleBlock())
		return false;
	return true;
}

/***************************************/

bool Parser::validSimpleBlock() {
	std::vector<Tok*>::iterator tmp = it;
	std::vector<Token*> backup = _tokens;
	if ((*it)->type == TokenType::End)
		return false;

	std::cout << "\nTesting validSimpleBlock at: " << (*it)->content << std::endl;
	if (simple_block_case1())
		return true;
	log("\t!!! NOT SIMPLE_BLOCK_CASE1");
	_tokens.clear();
	_tokens = backup;
	it = tmp;

	return false;
}

// [name] [parameter] '{' <simple_dir_lst> '}'
bool Parser::simple_block_case1() {
	std::cout << "\tEntered simple_block_case1 with: " << (*it)->content << std::endl;

	if ((*it)->type != TokenType::Name && (*it)->type != TokenType::Parameter)
		return false;
	std::string n = (*it)->content;
	it++;
	if ((*it)->type != TokenType::Name && (*it)->type != TokenType::Parameter)
		return false;
	std::string p = (*it)->content;
	it++;
	_tokens.push_back(new PreblockToken(n, p));
	if ((*it)->type != TokenType::OpenBrack)
		return false;
	_tokens.push_back(new OpenBracketToken);
	it++;
	if (!validSimpleDirectiveList())
		return false;
	std::cout << "\tRe-entered simple_block_case1 with: " << (*it)->content << std::endl;
	if ((*it)->type != TokenType::CloseBrack)
		return false;
	_tokens.push_back(new CloseBracketToken);
	it++;
	return true;
}

/***************************************/

bool Parser::validSimpleDirectiveList() {
	std::vector<Tok*>::iterator tmp = it;
	std::vector<Token*> backup = _tokens;
	if ((*it)->type == TokenType::End)
		return false;

	std::cout << "\nTesting validSimpleDirectiveList at: " << (*it)->content << std::endl;
	if (simple_directive_list_case1())
		return true;
	log("\t!!! NOT SIMPLE_DIRECTIVE_LIST_CASE1");
	_tokens.clear();
	_tokens = backup;
	it = tmp;
	if (simple_directive_list_case2())
		return true;
	log("\t!!! NOT SIMPLE_DIRECTIVE_LIST_CASE2");
	_tokens.clear();
	_tokens = backup;
	it = tmp;

	return false;
}

// <simple_dir> <simple_dir_lst>
bool Parser::simple_directive_list_case1() {
	std::cout << "\tEntered simple_directive_list_case1 with: " << (*it)->content << std::endl;

	if (!validSimpleDirective())
		return false;
	std::cout << "\tRe-entered simple_directive_list_case1 with: " << (*it)->content << std::endl;
	if (!validSimpleDirectiveList())
		return false;
	return true;
}

// <simple_dir>
bool Parser::simple_directive_list_case2() {
	std::cout << "\tEntered simple_directive_list_case2 with: " << (*it)->content << std::endl;

	if (!validSimpleDirective())
		return false;
	return true;
}

/***************************************/

bool Parser::validSimpleDirective() {
	std::vector<Tok*>::iterator tmp = it;
	std::vector<Token*> backup = _tokens;
	if ((*it)->type == TokenType::End)
		return false;

	std::cout << "\nTesting validSimpleDirective at: " << (*it)->content << std::endl;
	if (simple_directive_case1())
		return true;
	log("\t!!! NOT SIMPLE_DIRECTIVE_CASE1");
	_tokens.clear();
	_tokens = backup;
	it = tmp;

	return false;
}

// [name] <parameter_lst>
bool Parser::simple_directive_case1() {
	std::cout << "\tEntered simple_directive_case1 with: " << (*it)->content << std::endl;

	if ((*it)->type != TokenType::Name && (*it)->type != TokenType::Parameter)
		return false;
	_tokens.push_back(new NameToken((*it)->content));
	it++;

	if (!validParameterList())
		return false;
	std::cout << "\t\tSuccess: [Name] <parameter_lst>" << std::endl;
	return true;
}

/***************************************/

bool Parser::validParameterList() {
	std::vector<Tok*>::iterator tmp = it;
	std::vector<Token*> backup = _tokens;
	if ((*it)->type == TokenType::End)
		return false;

	std::cout << "\nTesting validParameterList at: " << (*it)->content << std::endl;
	if (parameter_list_case1())
		return true;
	log("\t!!! NOT PARAMETER_LIST_CASE1");
	_tokens.clear();
	_tokens = backup;
	it = tmp;
	if (parameter_list_case2())
		return true;
	log("\t!!! NOT PARAMETER_LIST_CASE2");
	_tokens.clear();
	_tokens = backup;
	it = tmp;

	return false;
}

// [parameter] ';'
bool Parser::parameter_list_case1() {
	std::cout << "\tEntered parameter_list_case1 with: " << (*it)->content << std::endl;

	if ((*it)->type != TokenType::Parameter)
		return false;
	_tokens.push_back(new ParameterToken((*it)->content));
	it++;
	if ((*it)->type != TokenType::Semicolon)
		return false;
	it++;
	std::cout << "\t\tSuccess: [Parameter] ';'" << std::endl;
	return true;
}

// [parameter] <parameter_lst>
bool Parser::parameter_list_case2() {
	std::cout << "\tEntered parameter_list_case2 with: " << (*it)->content << std::endl;

	if ((*it)->type != TokenType::Parameter)
		return false;
	_tokens.push_back(new ParameterToken((*it)->content));
	it++;
	if (!validParameterList())
		return false;
	std::cout << "\t\tSuccess: [Parameter] <parameter_lst>" << std::endl;
	return true;
}
