#include "../inc/Request.hpp"

Request::Request() {
	_port = htons(8080);
	_ip_address = inet_addr("127.0.0.1");
	_server_name = "localhost";
	_http_method = 0;
	_http_protocol = "HTTP/1.1";
	_is_CGI = false;
	_uri = "";
}

Request::~Request() {}

/***************************************/

void Request::setCGI(bool is_CGI) {
	_is_CGI = is_CGI;
}

void Request::setHTTPMethod(int http_method) {
	_http_method = http_method;
}

void Request::setURI(std::string uri) {
	_uri = uri;
}
