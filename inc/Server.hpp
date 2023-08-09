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
		uint32_t								getPort() const { return _port; }
		in_addr_t								getIPAddress() const { return _ip_address; }
		std::vector<std::string>				&getServerNames() { return _server_names; }
		std::map<int,std::vector<std::string> >	&getErrorPages() { return _error_pages; }
		size_t									getClientMaxBodySize() { return _client_max_body_size; }

		// Setters
		int setListen(std::list<Node>::iterator &it);
		int setServerName(std::list<Node>::iterator &it);
		int setLocationBlock(std::list<Node>::iterator &it);
		int setErrorPages(std::list<Node>::iterator &it);
		int setClientMaxBodySize(std::list<Node>::iterator &it);

		// Validity Checkers
		bool validDirective(std::string content);
		bool validHost(std::string ip);

	private:
		int										_server_fd;				// file descriptor of the server socket
		struct sockaddr_in						_socket_address;		// struct containing the address info of the server_socket

		uint32_t								_port;					// port specified by the 'listen' directive in config_file
		in_addr_t								_ip_address;			// IP address specified by the 'host' directive in config_file
		std::vector<std::string>				_server_names;			// server_names specified in the config_file. If this vector is empty, it means this is the default server block
		std::map<int,std::vector<std::string> >	_error_pages;			// map storing the error codes and their respective error_pages
		size_t									_client_max_body_size;	// lmax size (in bytes) of the client body according to config_file

};

#endif