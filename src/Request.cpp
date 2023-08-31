#include "../inc/Request.hpp"

Request::Request() {
	_port = 8080;
	_ip_address = inet_addr("127.0.0.1");
	_server_name = "localhost";
	_http_method = 0;
	_is_CGI = false;
	_is_error = false;
	_error_code = 404;
}

Request::~Request() {}

/***************************************/

void Request::setCGI(bool is_CGI) {
	_is_CGI = is_CGI;
}

void Request::setHTTPMethod(int http_method) {
	_http_method = http_method;
}
