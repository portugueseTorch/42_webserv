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
		int					getClientID() const { return _client_id; }

		// Setters
		void				setClientFD(int client_fd);
		void				setRequest(std::string request_str);

		Server				*parent_server;		// server who is serving the current client
		Location			*location_block;	// location block that will handle the request - NULL if
		Request				*request;			// object to be populated during the parseHTTPRequest()

		static int			num_clients;	// number of clients
	

	private:
		int					_client_id;		// ID of the client
		int					_client_fd;		// file descriptor of the client socket
		std::string			_request_str;	// request received from the client server

		std::string			_response;		// response to be sent

};

#endif
