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
	_events = NULL;
}

ServerEngine::~ServerEngine() {
	if (_events)
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
int ServerEngine::modifyEpoll(int fd, int operation, int flags) {
	// Create necessary struct to add fd to the interest set
	struct epoll_event ev;

	/* Add fd to epoll monitoring input, output, and in edge-triggered mode */
	ev.events = flags;
	ev.data.fd = fd;

	// Add fd and ev to the list of interest file descriptors
	if (operation == EPOLL_CTL_DEL) {
		if (epoll_ctl(_epoll_fd, operation, fd, NULL) == -1) {
			log(std::cerr, MsgType::ERROR, "epoll_ctl() call failed", "");
			return 1;
		}
	} else {
		if (epoll_ctl(_epoll_fd, operation, fd, &ev) == -1) {
			log(std::cerr, MsgType::ERROR, "epoll_ctl() call failed", "");
			return 1;
		}
	}
	if (operation == EPOLL_CTL_ADD)
		log(std::cout, MsgType::INFO, "Added to poll fd", std::to_string(fd));
	else if (operation == EPOLL_CTL_MOD)
		log(std::cout, MsgType::INFO, "Modified poll fd", std::to_string(fd));
	else if (operation == EPOLL_CTL_DEL)
		log(std::cout, MsgType::INFO, "Deleted poll fd", std::to_string(fd));
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
			if (errno == EADDRINUSE)
				continue;
			log(std::cerr, MsgType::ERROR, "listen() call failed", "");
			return 1;
		}
		std::string first_part = "Server " + std::to_string(it->getServerID()) + " is listening on port";
		log(std::cout, MsgType::INFO, first_part, std::to_string(ntohs(it->getPort())));
		_server_map[it->getServerFD()] = *it;
		if (modifyEpoll(it->getServerFD(), EPOLL_CTL_ADD, EPOLLIN | EPOLLET))
			return 1;
	}
	return 0;
}

int ServerEngine::assignServer(Client &client) {
	bool exact_match_found = false;
	std::vector<Server*> possible_servers;
	std::vector<Server*> backup_servers;

	/**	First check for the Port and IP Address of the request against each of the server block.
	 *	If an exact match is found add to possible_servers; if a non-exact match is found, and no
	 *	exact matches have been found so far, add to backup_servers.
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
			goto location_assignment;
		}
		// If multiple were found, try to get an exact match for the server_name directive
		for (std::vector<Server*>::iterator it = possible_servers.begin(); it != possible_servers.end(); it++) {
			if (std::find((*it)->getServerNames().begin(), (*it)->getServerNames().end(), client.request->getServerName()) != (*it)->getServerNames().end()) {
				client.parent_server = *it;
				goto location_assignment;
			}
		}
	} else {
		// If no appropriate server was found, the default server will handle the request
		if (backup_servers.size() == 0)
			goto location_assignment;
		// If multiple were found, try to get an exact match for the server_name directive
		for (std::vector<Server*>::iterator it = backup_servers.begin(); it != backup_servers.end(); it++) {
			if (std::find((*it)->getServerNames().begin(), (*it)->getServerNames().end(), client.request->getServerName()) != (*it)->getServerNames().end()) {
				client.parent_server = *it;
				goto location_assignment;
			}
		}
	}

	location_assignment:
		std::string client_id_string = "Client '" + std::to_string(client.getClientID()) + "' assigned to server with ID";
		log(std::cout, MsgType::SUCCESS, client_id_string, std::to_string(client.parent_server->getServerID()));
		client.parent_server->displayServer();

		// Iterate over all location blocks from the parent server. If a Location is found that matches URI, assign it
		for (std::vector<Location>::iterator it = client.parent_server->getLocations().begin(); it != client.parent_server->getLocations().end(); it++) {
			if (it->getLocation() == client.request->getURI()) {
				client.location_block = &(*it);
				break ;
			}
		}

		if (client.location_block) {
			client_id_string = "Client '" + std::to_string(client.getClientID()) + "' assigned to location block";
			log(std::cout, MsgType::SUCCESS, client_id_string, "");
			client.location_block->displayLocationBlock();
		} else {
			log(std::cout, MsgType::INFO, client_id_string, "NONE");
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
		log(std::cerr, MsgType::ERROR, "accept() call failed", "");
		return 1;
	}

	// Set up the client
	client.setClientFD(fd);
	if (client.setupClient())
		return 1;

	// Add client socket to epoll() interest list and client map
	if (modifyEpoll(fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLET))
		return 1;
	_client_map[fd] = client;
	return 0;
}

int ServerEngine::closeConnection(int fd) {
	// Delete fd from the epoll() interest group
	if (modifyEpoll(fd, EPOLL_CTL_DEL, 0))
		return 1;

	// Remove fd from its respective map
	if (_server_map.find(fd) != _server_map.end()) {
		_server_map.erase(fd);
		log(std::cout, MsgType::INFO, "Server removed from server map on fd", std::to_string(fd));
	}
	if (_client_map.find(fd) != _client_map.end()) {
		_client_map.erase(fd);
		log(std::cout, MsgType::INFO, "Client removed from client map on fd", std::to_string(fd));
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
	char buf[MAX_LENGTH];
	int	ret;
	int	fd = client.getClientFD();

	// Read message until we reach the end
	while (1) {
		ret = read(fd, &buf, MAX_LENGTH);
		if (ret == -1) {
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			else {
				log(std::cout, MsgType::ERROR, "read() call failed", "");
				return 1;
			}
		} else if (ret == 0) {
			log(std::cout, MsgType::WARNING, "Client closed the connection on fd", std::to_string(fd));
			closeConnection(client.getClientFD());
			return 0;
		}
	}

	// Update the client fd on epoll to be ready for writting
	if (modifyEpoll(fd, EPOLL_CTL_MOD, EPOLLOUT | EPOLLET))
		return 1;

	log(std::cout, MsgType::SUCCESS, "Message received on client socket", std::to_string(client.getClientFD()));
	std::cout << buf << std::endl;

	// Parse HTTP Request
	client.parseHTTPRequest(buf);

	// Assign the server according to the parsed request
	if (assignServer(client)) {
		log(std::cerr, MsgType::ERROR, "Failure assigning server", "");
		return 1;
	}

	return 0;
}

/**
 * @brief sends the regular HTML response to a regular HTML request
 * from CLIENT
 * 
 * @param client Client to which the request reffers
 * @return int Returns 0 on success, and 1 on failure
 */
int ServerEngine::sendRegResponse(Client &client) {
	std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 30\r\n\r\n<h1>Hello there, darling!</h1>\r\n";
	int ret = write(client.getClientFD(), response.c_str(), response.length());
	if (ret != response.length()) {
		if (ret == -1) {
			log(std::cerr, MsgType::ERROR, "send() call failed", "");
			// TODO: consider removing from the set and from the map
			return 1;
		} else
			log(std::cout, MsgType::WARNING, "Unable to send the full data", "");
	}

	// Reset client fd on epoll() to listen to incoming connections instead
	if (modifyEpoll(client.getClientFD(), EPOLL_CTL_MOD, EPOLLIN | EPOLLET))
		return 1;

	return 0;
}

/**
 * @brief Runs the CGI script in a child process, writes the output to
 * a pipe, and the parent process reads from the pipe and sends the response
 * to the client. It assumes that all requests fed to the function are valid
 * 
 * @param client Client to which the request reffers
 * @return int Returns 0 on success, and 1 on failure
 */
int ServerEngine::sendCGIResponse(Client &client) {
	int pipe_fd[2];
	int pid;

	// Create the pipe
	if (pipe(pipe_fd) == -1) {
		log(std::cerr, MsgType::ERROR, "pipe() call failed", "");
		return 1;
	}
	// Fork and execve the script on the child pr
	if ((pid = fork()) == -1) {
		log(std::cerr, MsgType::ERROR, "fork() call failed", "");
		return 1;
	}

	// In the child process, execute the cgi script
	if (pid == 0) {
		std::cout << "Hello, there!" << std::endl;
		dup2(pipe_fd[1], STDOUT_FILENO);
		close(pipe_fd[0]);
		char *args[] = { (char *)"/usr/bin/python3", (char *)"cgi-bin/cgi.py", NULL };
		execve("/usr/bin/python3", args, NULL);
	} else {
		close(pipe_fd[1]);
		wait(NULL);
		char *msg = new char[256];
		if (read(pipe_fd[0], msg, 256) == -1) {
			log(std::cerr, MsgType::ERROR, "read() call failed", "");
			return 1;
		}
		std::cout << msg << std::endl;
	}
	return 0;
}

/**
 * @brief Builds an error response and sends it back to the client
 * 
 * @param client Client to whom the response will be sent
 * @return int Returns 0 on success, and 1 on failure
 */
int ServerEngine::sendErrResponse(Client &client) {

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
	// Attempt to build a response
	if (client.buildHTTPResponse())
		return 1;

	// If it's an error, call sendErrResponse()
	if (client.request->getIsError()) {
		if (sendErrResponse(client))
			return 1;
		return 0;
	}

	// If the request is for CGI
	if (client.request->getIsCGI()) {
		if (sendCGIResponse(client))
			return 1;
	} else {
		if (sendRegResponse(client))
			return 1;
	}
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
					log(std::cout, MsgType::INFO, "Change of status in fd", std::to_string(_events[i].data.fd));
					if (readHTTPRequest(_client_map[_events[i].data.fd]))
						return 1;
				}
			}

			// Write event
			else if (_events[i].events & EPOLLOUT) {
				// Send response
				if (sendResponse(_client_map[_events[i].data.fd]))
					return 1;
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
