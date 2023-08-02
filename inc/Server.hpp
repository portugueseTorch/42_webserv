#pragma once
#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <vector>
# include <algorithm>
# include <sys/socket.h>
# include <arpa/inet.h>

class Server {
	public:


	private:
		int					_server_fd;			// file descriptor of the server socket
		in_port_t			_port;				// port to be bound to the server_socket
		std::string			_ip_address;		// IP address to be bound to the server_socket
		struct sockaddr_in	_socket_address;	// struct containing the address info of the server_socket
};

#endif