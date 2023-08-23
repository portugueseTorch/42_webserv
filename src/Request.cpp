#include "../inc/Request.hpp"

Request::Request() {
	_port = 8080;
	_ip_address = inet_addr("127.0.0.1");
	_server_name = "localhost";
	_http_method = Methods::GET;
}

Request::~Request() {}
