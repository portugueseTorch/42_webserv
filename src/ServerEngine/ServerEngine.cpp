#include "../inc/ServerEngine.hpp"

std::vector<std::string> ServerEngine::directives = {
	"listen",
	"server_name",
	"root",
	"index",
	"error_page",
	"location",
	"client_max_body_size",
	"fastcgi_pass",
	"autoindex",
	"http_method",
};

ServerEngine::ServerEngine(std::list<Node> nodes) {
	_nodes = nodes;
	_num_servers = 0;
}

ServerEngine::~ServerEngine() {}

/**
 * @brief Handles invalid directives
 * 
 * @param it reference to an iterator for a list of nodes containing the invalid input
 */
void ServerEngine::handleInvalidInput(std::list<Node>::iterator &it) {
	int min_distance = INT32_MAX;
	std::string best_match;

	if (it->_type != NodeType::Name)
		return ;

	for (std::vector<std::string>::iterator i = ServerEngine::directives.begin(); i != ServerEngine::directives.end(); i++)
	{
		int distance = damerauLevenshteinDistance(it->_content, *i);
		if (distance < min_distance)
		{
			min_distance = distance;
			best_match = *i;
		}
	}
	if (min_distance != 0)
	{
		if (min_distance < 4) {
			log(std::cerr, MsgType::ERROR, "Unkown directive", it->_content);
			std::cerr << "       |------> Did you mean: " << best_match << "?" << std::endl;
		}
		else
			log(std::cerr, MsgType::ERROR, "Unknown directive", it->_content);
	}
}

/**
 * @brief Configures the server blocks according to the config_file by iterating
 * over the node list [_nodes] created by the parser
 * 
 * @return Returns 0 on success, and 1 on failure
 */
int ServerEngine::configureServers() {
	for (std::list<Node>::iterator it = _nodes.begin(); it != _nodes.end(); it++) {
		if (it->_type == NodeType::ServerBlock) {
			if (configureServer(it)) {
				handleInvalidInput(it);
				return 1;
			}
			_num_servers++;
			if (it == _nodes.end())
				break;
		}
	}
	return 0;
}

/**
 * @brief Attempts to setup each server in the Server vector [_servers]. If some, but not all
 * servers fail to setup up, the program continues, failing only if all fail to setup
 * 
 * @return Returns 0 if at least one server was successfully setup, and 1 otherwise 
 */
int ServerEngine::setupServers() {
	bool all_failed = true;
	for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		if (it->setupServer()) {
			std::string err_msg = "Failure setting up server block " + std::to_string(it->getServerID());
			log(std::cerr, MsgType::ERROR, err_msg, "");
		} else
			all_failed = false;
	}
	if (all_failed) {
		log(std::cerr, MsgType::FATAL, "Unable to boot servers", "");
		return 1;
	}
	return 0;
}

/**
 * @brief Displays the servers stored in [_servers]
 * 
 */
void ServerEngine::displayServers() {
	for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++)
		it->displayServer();
}

////////////////////////////////////////////////////////

/**
 * @brief Configures each server block storing the resulting Server in a vector
 * of Servers [_servers]
 * 
 * @param it reference to an iterator for a list of nodes
 * @return int 
 */
int ServerEngine::configureServer(std::list<Node>::iterator &it) {
	Server new_server;
	int bracks = 0;

	// skip the 'server' keyword and check and skip the '{'
	it++;
	if (it->_type == NodeType::OpenBracket) {
		bracks++;
		it++;
	} else {
		std::cerr << LIGHT_RED << "[ERROR]:\t\tFailure setting up server" << RESET << std::endl;
		return 1;
	}

	// setup each individual server
	for (; it != _nodes.end(); it++) {
		if (bracks == 0) {
			it--;
			break ;
		}
		if (it->_type == NodeType::OpenBracket)
			bracks++;
		else if (it->_type == NodeType::CloseBracket)
			bracks--;
		else if (it->_type == NodeType::Name) {
			if (new_server.handleName(it))
				return 1;
		} else if (it->_type == NodeType::LocationBlock) {
			if (new_server.handleLocationBlock(it))
				return 1;
		} else if (it->_type == NodeType::ServerBlock || it->_type == NodeType::LocationBlock) {
			it--;
			break;
		}
	}
	_servers.push_back(new_server);
	return 0;
}
