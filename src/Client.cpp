#include "../inc/Client.hpp"

int Client::num_clients = 0;

Client::Client() {
	num_clients++;
	_client_fd = -1;
	parent_server = NULL;
	location_block = NULL;
	request = NULL;
	response = NULL;
	_client_id = num_clients;
	_status_code = 0;
	_last_exchange = time(NULL);
	toKill = false;
}

Client::~Client() {
	if (request)
		delete request;
	request = NULL;
}

void Client::reset() {
	_status_code = 0;
	location_block = NULL;
	if (request)
		delete request;
	request = NULL;
	if (response)
		delete response;
	response = NULL;
}

void Client::setClientFD(int client_fd) {
	_client_fd = client_fd;
}

void Client::setStatusCode(int status_code) {
	_status_code = status_code;
}

/**
 * @brief Sets _client_fd to be non-blocking
 * 
 * @return int Returns 0 on success, and 1 on failure
 */
int Client::setupClient() {
	// get current flags
	int flags = fcntl(_client_fd, F_GETFL, 0);
	if (flags == -1) {
		log(std::cerr, ERROR, "fcntl() call failed", "");
		return 1;
	}

	// add O_NONBLOCK to previous flags, and set them
	if (fcntl(_client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		log(std::cerr, ERROR, "fcntl() call failed", "");
		return 1;
	}

	std::stringstream ss;
	ss << ntohs(parent_server->getPort());
	log(std::cout, SUCCESS, "New Client set up on port", ss.str());
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
	if (!request) {
		request = new HTTPRequest;
		if (!parent_server) {
			log(std::cout, FATAL, "Parent server is not assiged", "");
			return 1;
		}
		request->setPort(parent_server->getPort());
		request->setIPAddress(parent_server->getIPAddress());
	}
	request->process(request_str);
	if (request->fullyParsed && !request->success()) {
		_status_code = request->getStatusCode();
		return 1;
	}
	return 0;
}

void Client::updateTime() {
	_last_exchange = time(NULL);
}

int	Client::sendTimeoutMessage() {
	std::string msg = "HTTP/1.1 408 Request Timeout\r\n\
Server: Webserv/42.0\r\nContent-Type: text/plain\r\n\
Content-Length: 21\r\n\r\n408 Request Timeout\r\n";
	if (send(_client_fd, msg.c_str(), msg.length(), 0) == -1) {
		log(std::cerr, ERROR, "send() call failed", "");
		return 1;
	}
	std::cout << "Timeout message sent\n" << std::endl;
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
	if (request)
		log(std::cout, INFO, "Requested URI", request->getRequestURI());
	response = new HTTPResponse(request, parent_server);
	if (toKill) response->toKill = true;
	response->build();
	return 0;
}
