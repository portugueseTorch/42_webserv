#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include "Webserv.hpp"
#include "HTTPRequest/Parser.hpp"

// enum RequestMethod {
// 	GET,
// 	POST,
// 	DELETE
// };
// Documentation: https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages

/* 
Bodies can be broadly divided into two categories:

Single-resource bodies, consisting of one single file, defined by the two headers: Content-Type and Content-Length.
Multiple-resource bodies, consisting of a multipart body, each containing a different bit of information. This is typically associated with HTML Forms.
 */
class HTTPBody;

class HTTPParser;

class HTTPRequest {
	public:
		HTTPRequest(std::string request);
		~HTTPRequest();

	private:
		HTTPRequest(){};

		int parse();
		int tokenize();
		int setup();
		void displayRequest();
		void displayParsedRequest();

		HTTPParser * parser;
		std::string _content;
		std::string _method;
		std::string	_requestURI;
		std::map<std::string, std::string> _params;
		// std::list<Node>::const_iterator it;
		// std::list<Node>::const_iterator end;

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

};

#endif