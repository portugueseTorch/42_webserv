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

int ServerEngine::configureServers() {
	for (std::list<Node>::iterator it = _nodes.begin(); it != _nodes.end(); it++) {
		if (it->_type == NodeType::ServerBlock) {
			if (setupServer(it)) {
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

void ServerEngine::displayServers() {
	std::vector<Server>::iterator it = _servers.begin();
	for (int i = 1; it != _servers.end(); it++, i++)
		it->displayServer(i);
}

////////////////////////////////////////////////////////

int ServerEngine::setupServer(std::list<Node>::iterator &it) {
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
	// std::cout << it->_type << std::endl;
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
			if (new_server.handleLocationBlock(it)) //TODO: FINISH THIS
				return 1;
		} else if (it->_type == NodeType::ServerBlock) {
			it--;
			break;
		}
	}
	_servers.push_back(new_server);
	return 0;
}
