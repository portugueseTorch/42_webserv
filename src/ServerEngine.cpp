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
	for (int i = 1; it != _servers.end(); it++, i++) {
		std::cout << "[Server " << i << "]\n";
		std::cout << "\tPort: " << it->getPort() << "\n";
		std::cout << "\tHost: " << it->getIPAddress() << "\n";

		std::cout << "\tServer Names: ";
		std::vector<std::string> server_names = it->getServerNames();
		for (std::vector<std::string>::iterator i = server_names.begin(); i != server_names.end(); i++)
			std::cout << *i << " ";
		std::cout << std::endl;

		std::cout << "\tError Pages:\n";
		std::map<int,std::vector<std::string> > error_pages = it->getErrorPages();
		for (std::map<int,std::vector<std::string> >::iterator i = error_pages.begin(); i != error_pages.end(); i++) {
			std::cout << "\t  - [ " << (*i).first << ", ";
			for (std::vector<std::string>::iterator j = (*i).second.begin(); j != (*i).second.end(); j++)
				std::cout << *j << " ";
			std::cout << "]\n";
		}

		std::cout << "\tClient max body size: " << it->getClientMaxBodySize() << std::endl;;
	}
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
		if (bracks == 0)
			break ;
		if (it->_type == NodeType::OpenBracket)
			bracks++;
		else if (it->_type == NodeType::CloseBracket)
			bracks--;
		else if (it->_type == NodeType::Name) {
			if (handleName(new_server, it))
				return 1;
		} else if (it->_type == NodeType::LocationBlock) {
			if (handleLocationBlock(new_server, it)) //TODO: FINISH THIS
				return 1;
		} else if (it->_type == NodeType::ServerBlock) {
			it--;
			break;
		}
	}
	_servers.push_back(new_server);
	return 0;
}

/*************************************/
/************** HANDLERS *************/
/*************************************/

int ServerEngine::handleName(Server &new_server, std::list<Node>::iterator &it) {
	if (!new_server.validDirective(it->_content))
			return 1;

	if (it->_content == "listen") {
		if (new_server.setListen(++it))
			return 1;
	} else if (it->_content == "server_name") {
		if (new_server.setServerName(++it))
			return 1;
	} else if (it->_content == "root") {
		//TODO: FINISH THIS;
	} else if (it->_content == "error_page") {
		if (new_server.setErrorPages(++it))
			return 1;
	} else if (it->_content == "client_max_body_size") {
		if (new_server.setClientMaxBodySize(++it))
			return 1;
	} else
		return 1;
	return 0;
}

int ServerEngine::handleLocationBlock(Server &new_server, std::list<Node>::iterator &it) {
	//TODO: FINISH THIS
	if (new_server.setLocationBlock(it))
		return 1;
	return 0;
}

