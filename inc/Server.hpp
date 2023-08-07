#pragma once
#ifndef SERVER_HPP
# define SERVER_HPP

# include "Webserv.hpp"

class Server {
	public:
		Server();
		~Server();
		void bootServer(std::string port, std::string ip_address);

		// Getters
		uint32_t getPort() const { return _port; }
		std::string getServerName() const { return _server_name; }
		in_addr_t getIPAddress() const { return _ip_address; }

		// Setters
		int setListen(std::string param);
		int setIPAddress(std::string ip_address);
		int setServerName(std::string server_name);

		// Validity Checkers
		bool validDirective(std::string content);
		bool validListen(std::string param);
		bool validHost(std::string ip);

	private:
		int					_server_fd;			// file descriptor of the server socket
		struct sockaddr_in	_socket_address;	// struct containing the address info of the server_socket

		uint32_t			_port;				// port specified by the 'listen' directive in config_file
		std::string			_server_name;		// server_name specified in the config_file
		in_addr_t			_ip_address;		// IP address specified by the 'host' directive in config_file

};

#endif