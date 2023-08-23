#include "../inc/ServerEngine.hpp"
#include <cstdlib>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

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

ServerEngine::~ServerEngine() {
	delete[] _events;
}

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
		log(std::cerr, MsgType::ERROR, "could not setup server", "");
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
 * @brief Checks if file descriptos FD is a client by attempting to find it in
 * the client map
 * 
 * @param fd File descriptor to evaluate
 * @return Returns true if it's a client, and false otherwise
 */
bool ServerEngine::isClient(int fd) {
	return _client_map.find(fd) != _client_map.end();
}

/**
 * @brief Adds file descriptor FD to the interest group of epoll
 * 
 * @param fd File descriptor to add to epoll
 * @return Returns 0 on success, and 1 otherwise
 */
int ServerEngine::addToPoll(int fd, int flags) {
	// Create necessary struct to add fd to the interest set
	struct epoll_event ev;

	/* Add fd to epoll monitoring input, output, and in edge-triggered mode */
	ev.events = flags;
	ev.data.fd = fd;

	// Add fd and ev to the list of interest file descriptors
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		log(std::cerr, MsgType::ERROR, "epoll_ctl() call failed", "");
		return 1;
	}

	log(std::cout, MsgType::INFO, "Added to poll fd", std::to_string(fd));
	return 0;
}

/**
 * @brief Sets up the epoll instance and iterates over the vector of servers
 * adding each one to the epoll() interest group
 * 
 * @return Returns 0 on sucess, and 1 on failure
 */
int ServerEngine::setupEpoll() {
	// Setup _epoll_fd and _events
	_epoll_fd = epoll_create1(0);
	_events = new struct epoll_event[MAX_EVENTS];
	
	// Finish preparing the servers for connection (listen()) and add to interest poll
	for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		if (listen(it->getServerFD(), 10) == -1) {
			log(std::cerr, MsgType::ERROR, "listen() call failed", "");
			return 1;
		}
		std::string first_part = "Server " + std::to_string(it->getServerID()) + " is listening on port";
		log(std::cout, MsgType::INFO, first_part, std::to_string(ntohs(it->getPort())));
		_server_map[it->getServerFD()] = *it;
		if (addToPoll(it->getServerFD(), EPOLLIN))
			return 1;
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

	if ((fd = accept(server.getServerFD(), (sockaddr *) &client_address, &client_addr_len)) == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			log(std::cerr, MsgType::ERROR, "accept() call failed", "");
			return 1;
		}
	}

	// Set non-blocking file descriptor
	int flags = fcntl(fd, F_GETFL, 0);	// get current flags
	if (flags == -1) {
		log(std::cerr, MsgType::ERROR, "fcntl() call failed", "");
		return 1;
	}

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {	// add O_NONBLOCK to previous flags, and set them
		log(std::cerr, MsgType::ERROR, "fcntl() call failed", "");
		return 1;
	}

	// Add client socket to epoll() interest list
	if (addToPoll(fd, EPOLLIN))
		return 1;
	
	// client.setClientFD(fd);

	return 0;
}

int ServerEngine::readHTTPRequest(Client &client) {
	char buf[MAX_LENGTH];
	int	ret;
	int	fd = client.getClientFD();

	// Read message until we reach the end
	while (1) {
		ret = read(fd, &buf, MAX_LENGTH);
		if (ret == -1) {
			if (errno != EWOULDBLOCK && errno != EAGAIN)
				break;
			else {
				log(std::cout, MsgType::ERROR, "read() call failed", "");
				return 1;
			}
		}
	}

	log(std::cout, MsgType::SUCCESS, "Message received on client socket", std::to_string(client.getClientFD()));
	std::cout << buf << std::endl;
	return 0;
}

int ServerEngine::runServers() {
	// Setup epoll and add servers to _server_map
	if (setupEpoll())
		return 1;

	// Run the infinite loop
	int ready_fds;
	while (1) {
		// Call epoll_wait and see if it fails
		if ((ready_fds = epoll_wait(_epoll_fd, _events, MAX_EVENTS, EPOLL_TIMEOUT)) == -1) {
			log(std::cerr, MsgType::ERROR, "epoll_wait() call failed", "");
			return 1;
		}
		
		// Iterate over the ready file_descriptors and handle them appropriately
		for (int i = 0; i < ready_fds; i++) {
			// Read event
			std::cout << "I'm an event: " << _events[i].data.fd << std::endl;
			if (_events[i].events & EPOLLIN) {
				// If the fd is from an existing server, accept new connection
				if (isServer(_events[i].data.fd)) {
					log(std::cout, MsgType::INFO, "Incoming connection coming through fd", std::to_string(_events[i].data.fd));
					if (acceptNewConnection(_server_map[_events[i].data.fd]))
						return 1;
				}

				// Otherwise, read the request from the client
				else if (isClient(_events[i].data.fd)) {
					log(std::cout, MsgType::INFO, "Request coming through fd", std::to_string(_events[i].data.fd));
					// if (readHTTPRequest(_client_map[_events[i].data.fd]))
					// 	return 1;
				}
			}

			// Write event
			else if (_events[i].events & EPOLLOUT) {
				// Send message to client
			}
			
			// Error
			else if (_events[i].events & (EPOLLERR | EPOLLHUP)) {
				log(std::cerr, MsgType::ERROR, "file descriptor", std::to_string(_events[i].data.fd));
				// TODO: Consider removing the server where the error happened from the set
				continue;
			}
		}
	}
	return 0;
}
