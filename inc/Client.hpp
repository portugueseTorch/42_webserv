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

		int					parseHTTPRequest(std::string request_str);
		int					buildHTTPResponse();

		// Getters
		int					getClientFD() const { return _client_fd; }

		// Setters
		void				setClientFD(int client_fd);
		void				setRequest(std::string request_str);

		Server				*parent_server;	// server who is serving the current client
		Request				*request;		// object to be populated during the parseHTTPRequest()				

	private:
		int					_client_fd;		// file descriptor of the client socket
		std::string			_request_str;	// request received from the client server

		std::string			_response;		// response to be sent

};

#endif
