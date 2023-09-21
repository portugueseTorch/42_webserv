#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

# include "Webserv.hpp"

class Server;
class Location;
class HTTPRequest;

class HTTPResponse
{
	public:
		HTTPResponse(HTTPRequest *request, Server *parent_server);
		~HTTPResponse();

		std::string		getResponse() const { return _response; }
		bool			getIsError() const { return _status_code >= 400 && _status_code <= 518; }

		int				build();						// builds the HTTP response

		Server			*parent_server;					// parent server of the client
		Location		*location_block;				// location block of the client
		HTTPRequest		*request;						// relevant request

	private:
		std::string		getTime();						// returns a string with the current time formatted for the header
		std::string		getContentType(std::string);	// returns a string with the type of the content to be server
		std::string		getRelevantRoot();				// returns a string with the root that is relevant to the request
		void			assignLocationBlock();			// assigns the relevant location block (if any)
		void			readContent(std::string);		// attempts to read the requested file, setting status codes appropriately
		void			searchContent(std::string);		// searches the requested content
		int 			handleContentHeaders();			// handles content headers like content-type, content-length, and builds body

		std::string		_response;						// response to be sent
		std::string		_protocol;						// protocol of the request
		std::string		_time;							// current time formatted for the header
		std::string		_server;						// server header
		std::string		_content_type;					// type of the content that will be served
		std::string		_content_length;				// length of the content to be served
		std::string		_body;							// holds the contents of the requested content, if any
		int				_body_length;					// length of the body (includes the terminating "\r\n")
		int				_status_code;					// status code of the request
};

#endif
