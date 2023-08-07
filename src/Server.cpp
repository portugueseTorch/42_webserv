#include "../inc/Server.hpp"

Server::Server() {
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

/*************************************/
/************** SETTERS **************/
/*************************************/

int Server::setListen(std::string param) {
	if (!validListen(param))
			return 1;

	std::vector<std::string> split;
	int end = param.find(":"); 
	while (end != -1) { // Loop until no delimiter is left in the string.
		split.push_back(param.substr(0, end));
		param.erase(param.begin(), param.begin() + end + 1);
		end = param.find(":");
	}
	split.push_back(param.substr(0, end));

	if (split.size() == 2) {
		_ip_address = inet_addr(split[0].c_str());
		_port = std::stoi(split[1]);
	}

	return 0;
}

int Server::setServerName(std::string server_name) {
	_server_name = server_name;
	return 0;
}

int Server::setIPAddress(std::string ip_address) {
	if (!validHost(ip_address))
			return 1;
	
	if (ip_address == "localhost")
		ip_address = "127.0.0.1";
	_ip_address = inet_addr(ip_address.c_str());
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

bool Server::validListen(std::string param) {
	int flag = 0;

	for (size_t i = 0; i < param.length(); i++) {
		if (param[i] == ':')
			flag++;
		if (!(isdigit(param[i]) || param[i] == ':' || param[i] == '.')) {
			log(std::cerr, MsgType::ERROR, "Invalid listen parameter", param);
			return false;
		}
	}
	if (flag > 1)
		return false;

	if (flag) {
		std::vector<std::string> split;
		int end = param.find(":"); 
		while (end != -1) { // Loop until no delimiter is left in the string.
			split.push_back(param.substr(0, end));
			param.erase(param.begin(), param.begin() + end + 1);
			end = param.find(":");
		}
		split.push_back(param.substr(0, end));

		if (split[0] == "localhost")
			split[0] = "127.0.0.1";
		struct sockaddr_in sockaddr;
		if (inet_pton(AF_INET, split[0].c_str(), &(sockaddr.sin_addr)) != 1) {
			log(std::cerr, MsgType::ERROR, "Invalid IP Address number", split[0]);
			return false;
		}

		int port = std::stoi(split[1]);
		if (port < 1 || port > 65636) {
			log(std::cerr, MsgType::ERROR, "Invalid port number", split[1]);
			return false;
		}

  		return true;
	}

	return true;
}

bool Server::validHost(std::string ip) {
	struct sockaddr_in sockaddr;
	if (inet_pton(AF_INET, ip.c_str(), &sockaddr.sin_addr) != 1)
		return false;
	return true;
}
