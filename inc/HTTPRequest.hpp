#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include "Webserv.hpp"

enum RequestMethod {
	GET,
	POST,
	DELETE
};

class HTTPRequest {
	public:
		HTTPRequest(std::string request);
		~HTTPRequest();

	private:
		HTTPRequest(){};

		int parse();
		int tokenize();
		int setup();

		std::string _content;

		/* 
			Header fields and values
			Content-Length
			Request method
			Response status code
			Transfer-Encoding
			HTTP version
			Request URL
			Message body
		 */

}

#endif