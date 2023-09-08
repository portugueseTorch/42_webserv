#pragma once
#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Webserv.hpp"
# include "Request.hpp"
# include "Server.hpp"

class Server;
class Request;

class Client
{
	public:
		Client();
		~Client();

		int					setupClient();
		void				reset();

		int					parseHTTPRequest(std::string request_str);
		int					searchRequestedContent(std::string content);
		int					buildHTTPResponse();
		std::string			statusCodeToMessage(int status_code);
		std::string			getContentType(std::string uri);

		// Getters
		int					getClientFD() const { return _client_fd; }
		int					getClientID() const { return _client_id; }
		int					getStatusCode() const { return _status_code; }
		bool				getIsError() const { return _status_code >= 400 && _status_code <= 511; }
		std::string			getResponse() const { return _response; }

		// Setters
		void				setClientFD(int client_fd);
		void				setRequest(std::string request_str);
		void				setStatusCode(int status_code);

		Server				*parent_server;		// server who is serving the current client
		Location			*location_block;	// location block that will handle the request - NULL if
		Request				*request;			// object to be populated during the parseHTTPRequest()

		static int			num_clients;	// number of clients
	

	private:
		int					_client_id;		// ID of the client
		int					_client_fd;		// file descriptor of the client socket
		std::string			_request_str;	// request received from the client server
		std::string			_uri;			// requested content after changes

		int					_status_code;	// status code associated with the request

		std::string			_file_buff;		// buffer to where the file will be read
		std::string			_file_type;		// type of the requested file
		std::string			_response;		// response to be sent

};

#endif
