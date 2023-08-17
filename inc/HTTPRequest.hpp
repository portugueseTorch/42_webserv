#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include "Webserv.hpp"

enum RequestMethod {
	GET,
	POST,
	DELETE
};
// Documentation: https://developer.mozilla.org/en-US/docs/Web/HTTP/Messages

// method, url, http-version
class StartLine; 

//case-insensitive string followed by a colon (':') and a value whose structure depends upon the header.
class HTTPHeader; 

//there needs to be an empty line in between

//Optional: GET usually doesn't need one
/* 
Bodies can be broadly divided into two categories:

Single-resource bodies, consisting of one single file, defined by the two headers: Content-Type and Content-Length.
Multiple-resource bodies, consisting of a multipart body, each containing a different bit of information. This is typically associated with HTML Forms.
 */
class HTTPBody;

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

};

#endif