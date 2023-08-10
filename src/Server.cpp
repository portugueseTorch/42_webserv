#include "../inc/Server.hpp"

Server::Server() {
	_port = 8080;
	_ip_address = inet_addr("0.0.0.0");
	_server_fd = -1;
}

Server::~Server() {
	if (_server_fd != -1)
		close(_server_fd);
}

void Server::bootServer(std::string port, std::string ip_address) {
	// _port = std::stoi(port);
	// _ip_address = ip_address;
	// _server_fd = socket(AF_INET, SOCK_STREAM, 0);
	(void) port;
	(void) ip_address;
}

void Server::displayServer(int i) {
	std::cout << "[Server " << i << "]\n";
		std::cout << "\tPort: " << getPort() << "\n";
		std::cout << "\tHost: " << getIPAddress() << "\n";

		std::cout << "\tServer Names: ";
		std::vector<std::string> server_names = getServerNames();
		for (std::vector<std::string>::iterator i = server_names.begin(); i != server_names.end(); i++)
			std::cout << *i << " ";
		std::cout << std::endl;

		std::cout << "\tError Pages:\n";
		std::map<int,std::vector<std::string> > error_pages = getErrorPages();
		for (std::map<int,std::vector<std::string> >::iterator i = error_pages.begin(); i != error_pages.end(); i++) {
			std::cout << "\t  - [ " << (*i).first << ", ";
			for (std::vector<std::string>::iterator j = (*i).second.begin(); j != (*i).second.end(); j++)
				std::cout << *j << " ";
			std::cout << "]\n";
		}

		std::cout << "\tClient max body size: " << getClientMaxBodySize() << std::endl;

		std::cout << "\tIndex: ";
		std::vector<std::string> index = getIndex();
		for (std::vector<std::string>::iterator i = index.begin(); i != index.end(); i++)
			std::cout << *i << " ";
		std::cout << std::endl;

		std::cout << "\tRoot: " << getRoot() << "\n";
		std::cout << "\tAutoindex: " << getAutoindex() << "\n";

		std::cout << "\tHTTP Methods: ";
		std::vector<int> http_methods = getHTTPMethod();
		for (std::vector<int>::iterator i = http_methods.begin(); i != http_methods.end(); i++)
			std::cout << *i << " ";
		std::cout << std::endl;
}

/*************************************/
/************** SETTERS **************/
/*************************************/

int Server::setListen(std::list<Node>::iterator &it) {
	std::vector<std::string> split;

	for (std::list<Node>::iterator tmp = it; tmp->_type != NodeType::CloseBracket && tmp->_type != NodeType::Name; tmp++)
		split.push_back(tmp->_content);

	if (split.size() != 1) {
		std::string tmp = "";
		for (std::vector<std::string>::iterator i = split.begin(); i != split.end(); i++) {
			tmp += *i;
			if (++i != split.end())
				tmp += " ";
			i--;
		}
		log(std::cerr, MsgType::ERROR, "Too many arguments for listen", tmp);
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
			int tmp = std::stoi(split[0]);
			if (tmp < 1 || tmp > 65636) {
				log(std::cerr, MsgType::ERROR, "Invalid port number", split[0]);
				return 1;
			}
			_port = tmp;
		} else {
			struct sockaddr_in sockaddr;
			if (inet_pton(AF_INET, split[0].c_str(), &(sockaddr.sin_addr)) != 1) {
				log(std::cerr, MsgType::ERROR, "Invalid IP Address number", split[0]);
				return 1;
			}
			_ip_address = inet_addr(split[0].c_str());
		}
		it--;
		return 0;
	} else if (split.size() == 2) {
		struct sockaddr_in sockaddr;
		if (inet_pton(AF_INET, split[0].c_str(), &(sockaddr.sin_addr)) != 1) {
			log(std::cerr, MsgType::ERROR, "Invalid IP Address number", split[0]);
			return 1;
		}

		for (int i = 0; i < split[1].length(); i++) {
			if (!isdigit(split[1][i])) {
				log(std::cerr, MsgType::ERROR, "Invalid port number", split[1]);
				return 1;
			}
		}
		int port = std::stoi(split[1]);
		if (port < 1 || port > 65636) {
			log(std::cerr, MsgType::ERROR, "Invalid port number", split[1]);
			return 1;
		}
		_ip_address = inet_addr(split[0].c_str());
		_port = port;
		it--;
		return 0;
	}
	log(std::cerr, MsgType::ERROR, "Invalid listen directive", param);
	return 1;
}

int Server::setServerName(std::list<Node>::iterator &it) {
	for (; it->_type == NodeType::Parameter; it++)
		_server_names.push_back(it->_content);
	it--;

	if (_server_names.size() < 1) {
		log(std::cerr, MsgType::ERROR, "Invalid number of arguments for", "listen");
		_server_names.clear();
		return 1;
	}
	return 0;
}

int Server::setLocationBlock(std::list<Node>::iterator &it) {
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
	if (split.size() != 2 || split[0] != "listen")
		return 1;
	
	std::string location = split[1];
	//TODO: FINISH THIS
	return 0;
}

int Server::setErrorPages(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == NodeType::Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check valid number of parameters
	if (stash.size() < 2) {
		log(std::cerr, MsgType::ERROR, "Too few arguments for", "error_page");
		return 1;
	}

	for (int i = 0; i < stash.size() - 1; i++) {
		// Check all digits
		for (int j = 0; j < stash[i].length(); j++) {
			if (!isdigit(stash[i][j])) {
				log(std::cerr, MsgType::ERROR, "Invalid error code", stash[i]);
				return 1;
			}
		}

		// Convert to int and check if it's between 400 and 599
		int code = stoi(stash[i]);
		if (code < 400 || code > 599) {
			log(std::cerr, MsgType::ERROR, "Invalid error code", stash[i]);
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

int Server::setClientMaxBodySize(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == NodeType::Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check there is only 1 argument specified for client_max_body_size
	if (stash.size() != 1) {
		log(std::cerr, MsgType::ERROR, "Invalid number of arguments for", "client_max_body_size");
		return 1;
	}

	// Check the argument is valid
	enum type { NUMBER, KILO, MEGA, GIGA, ERR };
	int flag = NUMBER;
	std::string body_size = stash.back();
	stash.clear();
	for (int i = 0; i < body_size.length(); i++) {
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
		log(std::cerr, MsgType::ERROR, "Invalid client_max_body_size", body_size);
		return 1;
	}

	// Adjust max body size according to the relevant flag
	size_t cmbs = (size_t) stoi(body_size);
	if (flag == KILO)
		cmbs *= 1000;
	else if (flag == MEGA)
		cmbs *= 1000000;
	else if (flag == GIGA)
		cmbs *= 1000000000;

	_client_max_body_size = cmbs;
	return 0;
}

int Server::setIndex(std::list<Node>::iterator &it) {
	for (; it->_type == NodeType::Parameter; it++)
		_index.push_back(it->_content);
	it--;

	// Check there is only 1 argument specified for client_max_body_size
	if (_index.size() < 1) {
		log(std::cerr, MsgType::ERROR, "Invalid number of arguments for", "index");
		_index.clear();
		return 1;
	}

	return 0;
}

int Server::setAutoindex(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == NodeType::Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check valid number of parameters
	if (stash.size() != 1) {
		log(std::cerr, MsgType::ERROR, "Too few arguments for", "autoindex");
		return 1;
	}

	if (stash.back() == "on") {
		_autoindex = true;
		return 0;
	} else if (stash.back() == "off") {
		_autoindex = false;
		return 0;
	}

	log(std::cerr, MsgType::ERROR, "Invalid autoindex parameter", it->_content);
	return 1;
}

int Server::setRoot(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == NodeType::Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check there is only 1 argument specified for client_max_body_size
	if (stash.size() != 1) {
		log(std::cerr, MsgType::ERROR, "Invalid number of arguments for", "root");
		return 1;
	}
	_root = stash.back();
	return 0;
}

int Server::setHTTPMethod(std::list<Node>::iterator &it) {
	for (; it->_type == NodeType::Parameter; it++) {
		if (it->_content == "GET")
			_http_method.push_back(Methods::GET);
		else if (it->_content == "POST")
			_http_method.push_back(Methods::POST);
		else if (it->_content == "DELETE")
			_http_method.push_back(Methods::DELETE);
		else if (it->_content == "HEAD")
			_http_method.push_back(Methods::HEAD);
		else if (it->_content == "PUT")
			_http_method.push_back(Methods::PUT);
		else {
			log(std::cerr, MsgType::ERROR, "Invalid argument for http_method", it->_content);
			_http_method.clear();
			return 1;
		}
	}
	it--;

	// Check valid number of parameters
	if (_http_method.empty()) {
		log(std::cerr, MsgType::ERROR, "Too few arguments for", "http_methods");
		return 1;
	}
	return 0;
}

/*************************************/
/************** IS_VALID *************/
/*************************************/

bool Server::validDirective(std::string content) {
	std::vector<std::string>::iterator it = ServerEngine::directives.begin();
	for (; it != ServerEngine::directives.end(); it++) {
		if (content == *it)
			return true;
	}
	return false;
}

bool Server::validHost(std::string ip) {
	struct sockaddr_in sockaddr;
	if (inet_pton(AF_INET, ip.c_str(), &sockaddr.sin_addr) != 1)
		return false;
	return true;
}

/*************************************/
/************** HANDLERS *************/
/*************************************/

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
	// } else if (it->_content == "http_method") {
	// 	std::string param = it->_content + " must be within a location block";
	// 	log(std::cerr, MsgType::ERROR, "Invalid directive", param);
	// 	return 1;
	// } else
	} else
		return 1;
	return 0;
}

int Server::handleLocationBlock(std::list<Node>::iterator &it) {
	//TODO: FINISH THIS
	if (setLocationBlock(it))
		return 1;
	return 0;
}

