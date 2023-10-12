#include "ServerEngine/ServerEngine.hpp"
#include <cstdlib>
#include <netinet/in.h>
// #include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

std::string ServerEngine::possibleDirectives[] = {
	"listen",
	"server_name",
	"root",
	"index",
	"error_page",
	"location",
	"client_max_body_size",
	"return",
	"autoindex",
	"http_method"
};

std::vector<std::string> ServerEngine::directives(ServerEngine::possibleDirectives, \
	ServerEngine::possibleDirectives + sizeof(ServerEngine::possibleDirectives) / sizeof(std::string));

int supported_sc[] = {
	200, 201, 202, 204,
	300, 301, 302, 303, 304,
	400, 403, 404, 405, 408, 406, 410, 411, 413, 414, 415,
	500, 501, 502, 504, 505
};

std::vector<int> ServerEngine::supported_status_codes(supported_sc, supported_sc + sizeof(supported_sc) / sizeof(int));

bool ServerEngine::isSupportedStatusCode(int code) {
	return std::find(supported_status_codes.begin(), supported_status_codes.end(), code) != supported_status_codes.end();
}

ServerEngine::ServerEngine(Parser *parser) {
	originalParser = parser;
	_nodes = parser->getNodes();
	_num_servers = 0;
	_max_fd = 0;
}

ServerEngine::~ServerEngine() {
	closeAllConnections();
	if (originalParser)
		delete originalParser;
	originalParser = NULL;
}

/**
 * @brief Handles invalid directives
 * 
 * @param it reference to an iterator for a list of nodes containing the invalid input
 */
void ServerEngine::handleInvalidInput(std::list<Node>::iterator &it) {
	int min_distance = INT32_MAX;
	std::string best_match;

	if (it->_type != Name)
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
			log(std::cerr, ERROR, "Unkown directive", it->_content);
			std::cerr << "       |------> Did you mean: " << best_match << "?" << std::endl;
		}
		else
			log(std::cerr, ERROR, "Unknown directive", it->_content);
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
		if (it->_type == ServerBlock) {
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
			std::stringstream out;
			out << it->getServerID();
			
			std::string err_msg = "Failure setting up server block " + out.str();
			log(std::cerr, ERROR, err_msg, "");
		} else
			all_failed = false;
	}
	if (all_failed) {
		log(std::cerr, FATAL, "Unable to boot servers", "");
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
	if (it->_type == OpenBracket) {
		bracks++;
		it++;
	} else {
		log(std::cerr, ERROR, "could not setup server", "");
		return 1;
	}

	// setup each individual server
	for (; it != _nodes.end(); it++) {
		if (bracks == 0) {
			it--;
			break ;
		}
		if (it->_type == OpenBracket)
			bracks++;
		else if (it->_type == CloseBracket)
			bracks--;
		else if (it->_type == Name) {
			if (new_server.handleName(it))
				return 1;
		} else if (it->_type == LocationBlock) {
			if (new_server.handleLocationBlock(it))
				return 1;
		} else if (it->_type == ServerBlock || it->_type == LocationBlock) {
			it--;
			break;
		}
	}
	_servers.push_back(new_server);
	return 0;
}

/**
 * @brief Checks if file descriptos FD is a server by attempting to find it in
 * the server map
 * 
 * @param fd File descriptor to evaluate
 * @return Returns true if it's a server, and false otherwise
 */
bool ServerEngine::isServer(int fd) {
	return _server_map.find(fd) != _server_map.end();
}

/**
 * @brief Assigns the client to the appropriate server
 * 
 * @param client Client to Assign 
 * @return int Returns 0 on success, and 1 on failure
 */
int ServerEngine::assignServer(Client &client) {
	bool exact_match_found = false;
	std::vector<Server*> possible_servers;
	std::vector<Server*> backup_servers;

	/**
	 * First check for the Port and IP Address of the request against each of the server block.
	 * If an exact match is found add to possible_servers; if a non-exact match is found, and no
	 * exact matches have been found so far, add to backup_servers.
	**/
	for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		if (client.request->getPort() == it->getPort() && \
			client.request->getIPAddress() == it->getIPAddress()) {
			if (!exact_match_found) exact_match_found = true;
			possible_servers.push_back(&(*it));
		}
		else if (!exact_match_found && (client.request->getPort() == it->getPort() || \
			client.request->getIPAddress() == it->getIPAddress())) {
			backup_servers.push_back(&(*it));
		}
	}

	// If exact matches were found, else if only exact matches were found
	if (exact_match_found) {
		//	If only one was find, assign it to the client and go to location assignment
		if (possible_servers.size() == 1) {
			client.parent_server = possible_servers[0];
			return 0;
		}
		// If multiple were found, try to get an exact match for the server_name directive
		for (std::vector<Server*>::iterator it = possible_servers.begin(); it != possible_servers.end(); it++) {
			if (std::find((*it)->getServerNames().begin(), (*it)->getServerNames().end(), client.request->getServerName()) != (*it)->getServerNames().end()) {
				client.parent_server = *it;
				return 0;
			}
		}
	} else {
		// If no appropriate server was found, the default server will handle the request
		if (backup_servers.size() == 0)
			return 0;
		// If multiple were found, try to get an exact match for the server_name directive
		for (std::vector<Server*>::iterator it = backup_servers.begin(); it != backup_servers.end(); it++) {
			if (std::find((*it)->getServerNames().begin(), (*it)->getServerNames().end(), client.request->getServerName()) != (*it)->getServerNames().end()) {
				client.parent_server = *it;
				return 0;
			}
		}
	}
	return 0;
}

/**
 * @brief Accepts new connections on server SERVER, creates a client,
 * sets it up, and adds it to the client map
 * 
 * @param server Server that received the connection
 * @return Returns 0 on success, and 1 otherwise
 */
int ServerEngine::acceptNewConnection(Server &server) {
	int fd;
	Client client;
	struct sockaddr	client_address;
	socklen_t		client_addr_len = sizeof(client_address);

	// Set the default server to handle the server
	client.parent_server = &server;

	if ((fd = accept(server.getServerFD(), (sockaddr *) &client_address, &client_addr_len)) == -1) {
		log(std::cerr, ERROR, "accept() call failed", "");
		return 1;
	}

	// Set up the client
	client.setClientFD(fd);
	if (client.setupClient())
		return 1;

	// Add client socket to epoll() interest list and client map
	modifySet(fd, READ_SET, ADD_SET);
	_client_map[fd] = client;
	return 0;
}

int ServerEngine::closeConnection(int fd) {
	std::stringstream ss;
	ss << fd;

	// Delete fd from the relevant set
	if (FD_ISSET(fd, &_read_set))
		modifySet(fd, READ_SET, DEL_SET);
	if (FD_ISSET(fd, &_write_set))
		modifySet(fd, WRITE_SET, DEL_SET);

	// Remove fd from its respective map
	if (_server_map.find(fd) != _server_map.end()) {
		_server_map.erase(fd);
		log(std::cout, WARNING, "Server removed from server map on fd", ss.str());
	}
	if (_client_map.find(fd) != _client_map.end()) {
		// Reset the client
		_client_map[fd].reset();
		_client_map.erase(fd);
		log(std::cout, WARNING, "Client removed from client map on fd", ss.str());
	}
	// Close file descriptor
	close(fd);
	return 0;
}

/**
 * @brief Reads the request from CLIENT, parses the HTTP request
 * building a Request object. The instance of epoll list associated
 * with client_fd is modified to be monitored for read events. If
 * read() returns 0, close connection with the client and remove fd
 * from the map and epoll list
 * 
 * @param client Client whose request is read and parsed
 * @return int Returns 0 on success, and 1 on failure
 */
int ServerEngine::readHTTPRequest(Client &client) {
	unsigned char buf[MAX_LENGTH + 1];
	int	ret;
	int	fd = client.getClientFD();

	std::stringstream ss;
	ss << fd;
	memset(buf, 0, MAX_LENGTH + 1);
	ret = read(fd, buf, MAX_LENGTH);
	if (ret == -1) {
		log(std::cout, ERROR, "read() call failed", "");
		closeConnection(fd);
		return 1;
	} else if (ret == 0) {
		log(std::cout, WARNING, "Client closed the connection on fd", ss.str());
		closeConnection(fd);
		return 0;
	}

	client.updateTime();

	if (client.request && client.request->fullyParsed)
		return 0;

	std::string toParse(reinterpret_cast<const char*>(buf), ret);
	client.parseHTTPRequest(toParse);
	if (client.request->fullyParsed)
		modifySet(fd, READ_SET, MOD_SET);
	return 0;
}

/**
 * @brief Evaluates the request associated with the client,
 * and redirects to either sendCGIResponse or sendRegularResponse
 * according to the type of request
 * 
 * @param client CLient whose request will be evaluated
 * @return int Returns 0 on success, and 1 on failure
 */
int ServerEngine::sendResponse(Client &client) {
	if (!client.toKill && assignServer(client)) {
		log(std::cerr, ERROR, "Failure assigning server", "");
		return 1;
	}
	if (client.buildHTTPResponse())
		return 1;

	if (send(client.getClientFD(), client.response->getResponse().c_str(), client.response->getResponseLength(), 0) == -1) {
		log(std::cerr, ERROR, "send() call failed", "");
		return 1;
	}

	client.updateTime();

	if (!client.toKill && client.request->getKeepAlive()) {
		modifySet(client.getClientFD(), WRITE_SET, MOD_SET);
		client.reset();
	} else {
		closeConnection(client.getClientFD());
	}

	return 0;
}

/**
 * @brief Applies the operation OP to the set specified by SET
 * with fd FD
 * 
 * @param fd File descriptor of the server to manipulate
 * @param set Set to manipulate
 * @param op Operation to perform
 */
void ServerEngine::modifySet(int fd, int set, int op) {
	fd_set &curr = (set == READ_SET) ? _read_set : _write_set;

	if (op == MOD_SET) {
		modifySet(fd, set, DEL_SET);
		modifySet(fd, !set, ADD_SET);
	} else if (op == ADD_SET) {
		FD_SET(fd, &curr);
	} else if (op == DEL_SET) {
		FD_CLR(fd, &curr);
	}

	// Update the max_fd, if needed
	if (fd > _max_fd)
		_max_fd = fd;
}

int ServerEngine::setupSets() {
	std::map<int,Server*> assigned_servers;
	FD_ZERO(&_read_set);
	FD_ZERO(&_write_set);

	// Finish preparing the servers for connection (listen()) and add to interest poll
	for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		// If server already assigned, continue; else, add it to the map
		if (assigned_servers.count(it->getPort()))
			continue;
		else
			assigned_servers[it->getPort()] = &(*it);

		if (listen(it->getServerFD(), 10) == -1) {
			log(std::cerr, ERROR, "listen() call failed", "");
			return 1;
		}

		std::stringstream ss;
		ss << it->getServerID();

		std::string first_part = "Server " + ss.str() + " is listening on port";
		ss.str("");
		ss.clear();
		ss << ntohs(it->getPort());
		log(std::cout, INFO, first_part, ss.str());
		_server_map[it->getServerFD()] = *it;

		modifySet(it->getServerFD(), READ_SET, ADD_SET);
	}

	return 0;
}

void ServerEngine::checkConnectionTimeouts() {
	for (std::map<int,Client>::iterator it = _client_map.begin(); it != _client_map.end(); it++) {
		if (time(NULL) - it->second.getLastExchange() >= CONNECTION_TIMEOUT) {
			std::stringstream ss;
			ss << it->second.getClientFD();
			log(std::cout, INFO, "Client timed out on fd", ss.str());
			modifySet(it->first, READ_SET, MOD_SET);
			it->second.toKill = true;
		}
	}
}

int ServerEngine::runServers() {
	struct timeval t;
	fd_set read_cpy;
    fd_set write_cpy;

	// Setup fd_sets and add servers to _server_map
	if (setupSets())
		return 1;

	// Run the infinite loop
	while (1) {
		t.tv_sec = 1;
		t.tv_usec = 0;
		read_cpy = _read_set;
		write_cpy = _write_set;

		if (select(_max_fd + 1, &read_cpy, &write_cpy, NULL, &t) == -1 ) {
			log(std::cerr, ERROR, "select() call failed", "");
			return 1;
		}

		std::stringstream ss;
		// Iterate over the file descriptors to check for ready file descriptors
		for (int fd = 0; fd <= _max_fd; fd++) {
			ss << fd;
			// If the fd is set on read_set
			if (FD_ISSET(fd, &read_cpy)) {
				// If it's a server, accept new connection
				if (_server_map.count(fd)) {
					log(std::cout, INFO, "Incoming connection coming through fd", ss.str());
					if (acceptNewConnection(_server_map[fd]))
						return 1;
				}
				// If it's a client, accept new connection
				else if (_client_map.count(fd)) {
					if (readHTTPRequest(_client_map[fd]))
						return 1;
				}
			} else if (FD_ISSET(fd, &write_cpy)) {
				if (_client_map.count(fd)) {
					if (sendResponse(_client_map[fd]))
						return 1;
				}
			}
			ss.str("");
			ss.clear();
		}
		ServerEngine::checkConnectionTimeouts();
	}
	return 0;
}

void	ServerEngine::closeAllConnections( void ) {

	if ( _client_map.size()) {
		std::map<int, Client>::iterator it = _client_map.begin();
		for (; it != _client_map.end(); it++) {
			close(it->first);
			it->second.~Client();
		}
	}

}
