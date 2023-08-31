#include "../inc/Client.hpp"

Client::Client() {
	_client_fd = -1;
	_response = "";
	parent_server = NULL;
	request = NULL;
}

Client::~Client() {
	delete request;
}

void Client::setClientFD(int client_fd) {
	_client_fd = client_fd;
}

void Client::setRequest(std::string request_str) {
	_request_str = request_str;
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

/**
 * @brief Parses the HTTP request and builds the Request object
 * with the appropriat attributes according to the parsing
 * 
 * @param request String with the contents of the HTTP request
 * @return int Returns 0 on success, and 1 on failure
 */
int Client::parseHTTPRequest(std::string request_str) {
	request = new Request;
	_request_str = request_str;
	return 0;
}

/**
 * @brief Builds the HTTP response based off of the attributes
 * populated during parseHTTPRequest() function in the Request
 * object
 * 
 * @return int Returns 0 on success, and 1 on failure
 */
int Client::buildHTTPResponse() {
	
	return 0;
}
