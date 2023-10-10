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
		int				getResponseLength() const { return _response_length; }
		bool			isError() const { return _status_code >= 400 && _status_code <= 518; }
		bool			kill;

		int				build();								// builds the HTTP response
		int				buildBody();							// builds the body of the HTTP response, if any

		Server			*parent_server;							// parent server of the client
		Location		*location_block;						// location block of the client
		HTTPRequest		*request;								// relevant request

	private:
		std::string		getTime();								// returns a string with the current time formatted for the header
		std::string		getContentType(std::string);			// returns a string with the type of the content to be server
		std::string		getRelevantRoot();						// returns a string with the root that is relevant to the request
		std::string		statusCodeToMessage();					// returns a string with the message related with the status code
		bool			validClientBodySize();					// checks if the length of the client body is within bounds
		bool			isAllowedMethod();						// checks if the current method is allowed
		void			assignLocationBlock();					// assigns the relevant location block (if any)
		void			readFile(std::string, struct stat *);	// reads the content of the argument file
		void			handle_autoindex();						// handles cases where autoindex is on
		void			searchContent();						// searches the requested content
		void			searchErrorContent();					// searches the appropriate error file, if needed
		int				readContent(std::string);				// attempts to read the requested file, setting status codes appropriately
		int				buildCGIResponse();						// builds the CGI response
		int				handleReturn();							// handles return statement of the location block
		char**			vectToArr(std::vector<std::string> vect);

		std::string		_response;								// response to be sent
		std::string		_protocol;								// protocol of the request
		std::string		_time;									// current time formatted for the header
		std::string		_server;								// server header
		std::string		_content_type;							// type of the content that will be served
		std::string		_body;									// holds the contents of the requested content, if any
		std::string		_last_modified;							// time when the file was last modified
		std::string		_file_path;								// path of the file to read
		std::string		_new_url;								// new URL specified by the return directive
		int				_body_length;							// length of the body (includes the terminating "\r\n")
		int				_header_length;							// length of the headers portion of the response
		int				_response_length;						// total length of the HTTP response
		int				_status_code;							// status code of the request
};

#endif
