#pragma once
#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Webserv.hpp"
# include "Server.hpp"

class Server;
class HTTPRequest;
class HTTPResponse;

class Client
{
	public:
		Client();
		~Client();
		int					setupClient();
		void				reset();
		bool				kill;
		int					parseHTTPRequest(std::string request_str);
		int					buildHTTPResponse();
		int					sendTimeoutMessage();
		int					getClientFD() const { return _client_fd; }
		int					getClientID() const { return _client_id; }
		int					getStatusCode() const { return _status_code; }
		bool				getIsError() const { return _status_code >= 400 && _status_code <= 511; }
		time_t				getLastExchange() const { return _last_exchange; }
		void				setClientFD(int client_fd);
		void				setStatusCode(int status_code);
		void				updateTime();

		Server				*parent_server;		// server who is serving the current client
		Location			*location_block;	// location block that will handle the request - NULL if
		HTTPRequest			*request;			// object to be populated during the parseHTTPRequest()
		HTTPResponse		*response;			// response object
		static int			num_clients;		// number of clients

	private:
		int					_client_id;			// ID of the client
		int					_client_fd;			// file descriptor of the client socket
		int					_status_code;		// status code associated with the request
		time_t				_last_exchange;		// time of the last exchange (either received or sent)
};

#endif
