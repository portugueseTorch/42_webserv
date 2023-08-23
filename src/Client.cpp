#include "../inc/Client.hpp"

Client::Client() {
	_client_fd = -1;
}

Client::~Client() {}

void Client::setClientFD(int client_fd) {
	_client_fd = client_fd;
}

void Client::setRequest(std::string request) {
	_request = request;
}

int Client::setupClient() {
	// Set socket to be non-blocking
	int flags = fcntl(_client_fd, F_GETFL, 0);	// get current flags
	if (flags == -1) {
		log(std::cerr, MsgType::ERROR, "fcntl() call failed", "");
		return 1;
	}

	if (fcntl(_client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {	// add O_NONBLOCK to previous flags, and set them
		log(std::cerr, MsgType::ERROR, "fcntl() call failed", "");
		return 1;
	}
	log(std::cout, MsgType::INFO, "New Client set up", std::to_string(_client_fd));
	return 0;
}
