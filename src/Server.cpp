#include "Server.hpp"

int Server::num_servers = 0;

Server::Server() {
	num_servers++;
	_port = htons(8080);
	_ip_address = inet_addr("0.0.0.0");
	_server_fd = -1;
	_client_max_body_size = 1000000;
	_autoindex = false;
	_server_id = num_servers;
	_is_setup = false;
	_root = "";
}

Server::~Server() {
	if (_server_fd != -1)
		close(_server_fd);
}

/**
 * @brief Sets up the necessary structs for the networking operations,
 * as well as requesting the necessary socket, binding it to a port and
 * IP, and allowing reutilization of ports for the server
 * 
 * @return Returns 0 on success, and 1 otherwise 
 */
int Server::setupServer() {
	// Create a socket for the server
	_server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_server_fd == -1) {
		log(std::cerr, ERROR, "Unable to create socket", "");
		return 1;
	}

	// Allow sockets to be reutilized with setsockopt
	int flag = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &flag, sizeof(flag)) == -1) {
		log(std::cerr, ERROR, "setsockopt() call failed", "");
		return 1;
	}

	// Set socket to be non-blocking
	int flags = fcntl(_server_fd, F_GETFL, 0);	// get current flags
	if (flags == -1) {
		log(std::cerr, ERROR, "fcntl() call failed", "");
		return 1;
	}

	if (fcntl(_server_fd, F_SETFL, flags | O_NONBLOCK) == -1) {	// add O_NONBLOCK to previous flags, and set them
		log(std::cerr, ERROR, "fcntl() call failed", "");
		return 1;
	}

	// Populate the sockaddr_in struct to be used by bind()
	_socket_address.sin_family = AF_INET;
	_socket_address.sin_port = _port;
	_socket_address.sin_addr.s_addr = _ip_address;
	memset(_socket_address.sin_zero, 0, sizeof(_socket_address.sin_zero));

	// Bind the socket to the specified port and IP address
	std::cout << "Binding server on port " << _port << " at IP " << _ip_address << std::endl;
	if (bind(_server_fd, (const sockaddr *) &_socket_address, sizeof(_socket_address)) == -1) {
		log(std::cerr, ERROR, "Unable to bind socket", "");
		return 1;
	}

	_is_setup = true;
	return 0;
}

/**
 * @brief Displays the parameters for the server block, including the
 * subparameters in its location blocks, if any
 * 
 */
void Server::displayServer() {
	std::cout << "[Server " << getServerID() << "]\n";
		std::cout << "\tPort: " << ntohs(getPort()) << "\n";
		std::cout << "\tHost: " << getIPAddress() << "\n";

		std::cout << "\tServer Names: [ ";
		std::vector<std::string> server_names = getServerNames();
		for (std::vector<std::string>::iterator i = server_names.begin(); i != server_names.end(); i++)
			std::cout << *i << " ";
		std::cout << "]" << std::endl;

		std::cout << "\tError Pages:\n";
		std::map<int,std::vector<std::string> > error_pages = getErrorPages();
		for (std::map<int,std::vector<std::string> >::iterator i = error_pages.begin(); i != error_pages.end(); i++) {
			std::cout << "\t  - [ " << (*i).first << ", ";
			for (std::vector<std::string>::iterator j = (*i).second.begin(); j != (*i).second.end(); j++)
				std::cout << *j << " ";
			std::cout << "]\n";
		}

		std::cout << "\tClient max body size: " << getClientMaxBodySize() << std::endl;

		std::cout << "\tIndex: [ ";
		std::vector<std::string> index = getIndex();
		for (std::vector<std::string>::iterator i = index.begin(); i != index.end(); i++)
			std::cout << *i << " ";
		std::cout << "]" << std::endl;

		std::cout << "\tRoot: " << getRoot() << "\n";
		std::cout << "\tAutoindex: " << getAutoindex() << "\n";

		std::cout << "\tHTTP Method: ";
		std::vector<int> http_Method = getHTTPMethod();
		for (std::vector<int>::iterator i = http_Method.begin(); i != http_Method.end(); i++)
			std::cout << *i << " ";
		std::cout << std::endl;

		std::cout << "\tIs setup: " << _is_setup << "\n";

		std::cout << "\tLocations:\n";
		std::vector<Location> locations = getLocations();
		for (std::vector<Location>::iterator i = locations.begin(); i != locations.end(); i++) {
			i->displayLocationBlock();
		}
}

/*************************************/
/************** SETTERS **************/
/*************************************/

/**
 * @brief Sets the port number and IP address for the server, storing
 * them in [_port] and [_ip_address], respectively.
 * Port number is stored as uint32_t by the htons() function. 
 * IP address is stored as in_addr_t by the inet_addr(), which converts
 * and IPV4 from the numbers-and-dots notation into binary data in
 * network byte order.
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Server::setListen(std::list<Node>::iterator &it) {
	std::vector<std::string> split;

	for (std::list<Node>::iterator tmp = it; tmp->_type != CloseBracket && tmp->_type != Name; tmp++)
		split.push_back(tmp->_content);

	if (split.size() != 1) {
		std::string tmp = "";
		for (std::vector<std::string>::iterator i = split.begin(); i != split.end(); i++) {
			tmp += *i;
			if (++i != split.end())
				tmp += " ";
			i--;
		}
		log(std::cerr, ERROR, "Too many arguments for listen", tmp);
		return 1;
	}

	split.clear();
	std::string param = it->_content;
	// Split param
	int end = param.find(":"); 
	while (end != -1) { // Loop until no delimiter is left in the string.
		split.push_back(param.substr(0, end));
		param.erase(param.begin(), param.begin() + end + 1);
		end = param.find(":");
	}
	split.push_back(param.substr(0, end));

	
	// Handle special cases
	if (split[0] == "localhost")
		split[0] = "127.0.0.1";
	else if (split[0] == "*")
		split[0] = "0.0.0.0";

	if (split.size() == 1) {
		bool port = true;

		// Check if we have IP or Port specified
		for (size_t i = 0; i < split[0].length(); i++) {
			if (split[0][i] == '.') {
				port = false;
				break ;
			}
		}

		// Check special cases
		if (split[0] == "localhost" || split[0] == "*")
			port = false;

		if (port) {
			int tmp = std::atoi(split[0].c_str());
			if (tmp < 1 || tmp > 65636) {
				log(std::cerr, ERROR, "Invalid port number", split[0]);
				return 1;
			} else if (tmp < 1024)
				log(std::cout, WARNING, "Ports under 1024 are only available to the superuser", "");
			_port = htons(tmp);
		} else {
			struct sockaddr_in sockaddr;
			if (inet_pton(AF_INET, split[0].c_str(), &(sockaddr.sin_addr)) != 1) {
				log(std::cerr, ERROR, "Invalid IP Address number", split[0]);
				return 1;
			}
			_ip_address = inet_addr(split[0].c_str());
		}
		it--;
		return 0;
	} else if (split.size() == 2) {
		struct sockaddr_in sockaddr;
		if (inet_pton(AF_INET, split[0].c_str(), &(sockaddr.sin_addr)) != 1) {
			log(std::cerr, ERROR, "Invalid IP Address number", split[0]);
			return 1;
		}

		for (size_t i = 0; i < split[1].length(); i++) {
			if (!isdigit(split[1][i])) {
				log(std::cerr, ERROR, "Invalid port number", split[1]);
				return 1;
			}
		}
		int port = std::atoi(split[1].c_str());
		if (port < 1 || port > 65636) {
			log(std::cerr, ERROR, "Invalid port number", split[1]);
			return 1;
		} else if (port < 1024)
			log(std::cout, WARNING, "Ports under 1024 are only available to the superuser", "");
		_ip_address = inet_addr(split[0].c_str());
		_port = htons(port);
		it--;
		return 0;
	}
	log(std::cerr, ERROR, "Invalid listen directive", param);
	return 1;
}

/**
 * @brief Sets the server names for the server block, storing the result
 * in the [_server_names] vector
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Server::setServerName(std::list<Node>::iterator &it) {
	for (; it->_type == Parameter; it++)
		_server_names.push_back(it->_content);
	it--;

	if (_server_names.size() < 1) {
		log(std::cerr, ERROR, "Invalid number of arguments for", "listen");
		_server_names.clear();
		return 1;
	}
	return 0;
}

/**
 * @brief Sets the error pages for the server block by iterating over
 * the list of nodes IT, storing the result in [_error_pages], a map
 * where the key is an integer representing the error code, and the
 * value is a vector of strings representing the respective pages
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Server::setErrorPages(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check valid number of parameters
	if (stash.size() < 2) {
		log(std::cerr, ERROR, "Too few arguments for", "error_page");
		return 1;
	}

	for (size_t i = 0; i < stash.size() - 1; i++) {
		// Check all digits
		for (size_t j = 0; j < stash[i].length(); j++) {
			if (!isdigit(stash[i][j])) {
				log(std::cerr, ERROR, "Invalid error code", stash[i]);
				return 1;
			}
		}

		// Convert to int and check if it's between 400 and 599
		int code = std::atoi(stash[i].c_str());
		if (code < 400 || code > 599) {
			log(std::cerr, ERROR, "Invalid error code", stash[i]);
			return 1;
		}

		// Check if the code already exists in the map, and if so check if the current
		// error_page is already there to avoid duplicates
		if (_error_pages.find(code) != _error_pages.end()) {
			if (std::find(_error_pages[code].begin(), _error_pages[code].end(), stash.back()) != _error_pages[code].end())
				continue;
		}
		_error_pages[code].push_back(stash.back());
	}
	return 0;
}

/**
 * @brief Sets the client_max_body_size for the server block by iterating
 * over IT, storing it as a size_t in [_client_max_body_size]. The functions
 * handles numbers, as well as the k, m, and g abbreviations (case-insensitive)
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Server::setClientMaxBodySize(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check there is only 1 argument specified for client_max_body_size
	if (stash.size() != 1) {
		log(std::cerr, ERROR, "Invalid number of arguments for", "client_max_body_size");
		return 1;
	}

	// Check the argument is valid
	enum type { NUMBER, KILO, MEGA, GIGA, ERR };
	int flag = NUMBER;
	std::string body_size = stash.back();
	stash.clear();
	for (size_t i = 0; i < body_size.length(); i++) {
		if (body_size[i] == 'k' || body_size[i] == 'K' || \
			body_size[i] == 'm' || body_size[i] == 'M' || \
			body_size[i] == 'g' || body_size[i] == 'G') {
			if (i != (body_size.length() - 1)) {
				flag = ERR;
				break;
			}
			if (body_size[i] == 'k' || body_size[i] == 'K')
				flag = KILO;
			else if (body_size[i] == 'm' || body_size[i] == 'M')
				flag = MEGA;
			else
				flag = GIGA;
		} else if (!isdigit(body_size[i])) {
			flag = ERR;
			break;
		}
	}

	if (flag == ERR) {
		log(std::cerr, ERROR, "Invalid client_max_body_size", body_size);
		return 1;
	}

	// Adjust max body size according to the relevant flag
	size_t cmbs = (size_t) std::atoi(body_size.c_str());
	if (flag == KILO)
		cmbs *= 1000;
	else if (flag == MEGA)
		cmbs *= 1000000;
	else if (flag == GIGA)
		cmbs *= 1000000000;

	_client_max_body_size = cmbs;
	return 0;
}

/**
 * @brief Sets the indexes for the server block by iterating over IT, storing
 * the result in [_index] as a vector of strings
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Server::setIndex(std::list<Node>::iterator &it) {
	for (; it->_type == Parameter; it++) {
		if (it->_content != "/" && it->_content[it->_content.length() - 1] == '/')
			_index.push_back(it->_content.substr(0, it->_content.length() - 1));
		else
			_index.push_back(it->_content);
	}
	it--;

	// Check there is only 1 argument specified for client_max_body_size
	if (_index.size() < 1) {
		log(std::cerr, ERROR, "Invalid number of arguments for", "index");
		_index.clear();
		return 1;
	}
	return 0;
}

/**
 * @brief Sets the autoindex for the server block directive by iterating
 * over IT, storing the result as a boolean in [_autoindex]
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Server::setAutoindex(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check valid number of parameters
	if (stash.size() != 1) {
		log(std::cerr, ERROR, "Too few arguments for", "autoindex");
		return 1;
	}

	if (stash.back() == "on") {
		_autoindex = true;
		return 0;
	} else if (stash.back() == "off") {
		_autoindex = false;
		return 0;
	}

	log(std::cerr, ERROR, "Invalid autoindex parameter", it->_content);
	return 1;
}

/**
 * @brief Sets the root for the server block by iterating over IT, storing
 * the result in [_root] as a string
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Server::setRoot(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check there is only 1 argument specified for client_max_body_size
	if (stash.size() != 1) {
		log(std::cerr, ERROR, "Invalid number of arguments for", "root");
		return 1;
	} else if (stash.back()[0] != '/') {
		log(std::cerr, ERROR, "Invalid root directive: must start with '/'", stash.back());
		return 1;
	}
	if (stash.back() != "/" && stash.back()[stash.back().length() - 1] == '/')
		_root = stash.back().substr(0, stash.back().length() - 1);
	else
		_root = stash.back();
	return 0;
}

/**
 * @brief Sets the HTTP Method allowed for the server block by iterating
 * over IT, storing the result in [_http_method] as a vector of ints
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Server::setHTTPMethod(std::list<Node>::iterator &it) {
	for (; it->_type == Parameter; it++) {
		if (it->_content == "GET")
			_http_method.push_back(GET);
		else if (it->_content == "POST")
			_http_method.push_back(POST);
		else if (it->_content == "DELETE")
			_http_method.push_back(DELETE);
		else if (it->_content == "HEAD")
			_http_method.push_back(HEAD);
		else if (it->_content == "PUT")
			_http_method.push_back(PUT);
		else {
			log(std::cerr, ERROR, "Invalid argument for http_method", it->_content);
			_http_method.clear();
			return 1;
		}
	}
	it--;

	// Check valid number of parameters
	if (_http_method.empty()) {
		log(std::cerr, ERROR, "Too few arguments for", "http_Method");
		return 1;
	}
	return 0;
}

/**
 * @brief Sets the location block for the server block, storing it in a vector
 * of Locations [_locations]
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Server::setLocationBlock(std::list<Node>::iterator &it) {
	Location location;
	std::string directive = it->_content;

	// Split directive
	std::vector<std::string> split;
	int end = directive.find(" "); 
	while (end != -1) { // Loop until no delimiter is left in the string.
		split.push_back(directive.substr(0, end));
		directive.erase(directive.begin(), directive.begin() + end + 1);
		end = directive.find(" ");
	}
	split.push_back(directive.substr(0, end));

	// Check if the location directive is valid
	if (split.size() != 2 || split[0] != "location") {
		log(std::cerr, ERROR, "Invalid location directive", it->_content);
		return 1;
	}
	
	// Add the location being evaluated
	location.setLocation(split[1]);

	it++;
	it++;
	int bracks = 1;
	for (; bracks != 0; it++) {
		if (it->_type == OpenBracket)
			bracks++;
		else if (it->_type == CloseBracket)
			bracks--;
		else if (it->_content == "root") {
			if (location.setRoot(++it))
				return 1;
		} else if (it->_content == "index") {
			if (location.setIndex(++it))
				return 1;
		} else if (it->_content == "error_page") {
			if (location.setErrorPages(++it))
				return 1;
		} else if (it->_content == "autoindex") {
			if (location.setAutoindex(++it))
				return 1;
		} else {
			return 1;
		}
	}

	it--;
	_locations.push_back(location);
	return 0;
}

/*************************************/
/************** IS_VALID *************/
/*************************************/

/**
 * @brief Checks if the CONTENT is a valid directive, according to the
 * Server::directives vector of strings
 * 
 * @param content string to check if it's a parameter
 * @return Returns true if CONTENT is a directive, and false otherwise
 */
bool Server::validDirective(std::string content) {
	std::vector<std::string>::iterator it = ServerEngine::directives.begin();
	for (; it != ServerEngine::directives.end(); it++) {
		if (content == *it)
			return true;
	}
	return false;
}

/**
 * @brief Checks if the IP is a valid host
 * 
 * @param ip string to check if it's a valid IP
 * @return Returns true if IP is a directive, and false otherwise
 */
bool Server::validHost(std::string ip) {
	struct sockaddr_in sockaddr;
	if (inet_pton(AF_INET, ip.c_str(), &sockaddr.sin_addr) != 1)
		return false;
	return true;
}

/*************************************/
/************** HANDLERS *************/
/*************************************/

/**
 * @brief Function to handle a name, calling one of the setters
 * according to the name being evaluated
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, and 1 on failure 
 */
int Server::handleName(std::list<Node>::iterator &it) {
	if (!validDirective(it->_content))
			return 1;

	if (it->_content == "listen") {
		if (setListen(++it))
			return 1;
	} else if (it->_content == "server_name") {
		if (setServerName(++it))
			return 1;
	} else if (it->_content == "root") {
		if (setRoot(++it))
			return 1;
	} else if (it->_content == "error_page") {
		if (setErrorPages(++it))
			return 1;
	} else if (it->_content == "client_max_body_size") {
		if (setClientMaxBodySize(++it))
			return 1;
	} else if (it->_content == "index") {
		if (setIndex(++it))
			return 1;
	} else if (it->_content == "autoindex") {
		if (setAutoindex(++it))
			return 1;
	} else if (it->_content == "http_method") {
		if (setHTTPMethod(++it))
			return 1;
	} else
		return 1;
	return 0;
}

/**
 * @brief Function to handle a location, calling one of the location setter
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, and 1 on failure 
 */
int Server::handleLocationBlock(std::list<Node>::iterator &it) {
	if (setLocationBlock(it))
		return 1;
	return 0;
}

