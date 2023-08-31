#pragma once
#ifndef SERVER_HPP
# define SERVER_HPP

# include "Webserv.hpp"

class Location;

class Server {
	public:
		Server();
		~Server();

		int setupServer();

		void displayServer();

		// Getters
		int										getServerFD() const { return _server_fd; }
		int										getServerID() const { return _server_id; }
		uint32_t								getPort() const { return _port; }
		in_addr_t								getIPAddress() const { return _ip_address; }
		size_t									getClientMaxBodySize() { return _client_max_body_size; }
		std::string								getRoot() { return _root; }
		bool									getAutoindex() { return _autoindex; }
		std::vector<std::string>				&getIndex() { return _index; }
		std::vector<std::string>				&getServerNames() { return _server_names; }
		std::map<int,std::vector<std::string> >	&getErrorPages() { return _error_pages; }
		std::vector<int>						&getHTTPMethod() { return _http_method; }
		std::vector<Location>					&getLocations() { return _locations; }

		// Setters
		int setListen(std::list<Node>::iterator &it);
		int setServerName(std::list<Node>::iterator &it);
		int setLocationBlock(std::list<Node>::iterator &it);
		int setErrorPages(std::list<Node>::iterator &it);
		int setClientMaxBodySize(std::list<Node>::iterator &it);
		int setIndex(std::list<Node>::iterator &it);
		int setAutoindex(std::list<Node>::iterator &it);
		int setRoot(std::list<Node>::iterator &it);
		int setHTTPMethod(std::list<Node>::iterator &it);

		// Validity Checkers
		bool validDirective(std::string content);
		bool validHost(std::string ip);

		int handleName(std::list<Node>::iterator &it);
		int handleLocationBlock(std::list<Node>::iterator &it);

		static int num_servers;

	private:
		int										_server_id;				// ID of the server for management purposes
		int										_server_fd;				// file descriptor of the server socket
		struct sockaddr_in						_socket_address;		// struct containing the address info of the server_socket
		bool									_is_setup;				// flag to signal if the server is setup

		uint32_t								_port;					// port specified by the 'listen' directive in config_file
		in_addr_t								_ip_address;			// IP address specified by the 'host' directive in config_file
		std::vector<std::string>				_server_names;			// server_names specified in the config_file. If this vector is empty, it means this is the default server block
		std::map<int,std::vector<std::string> >	_error_pages;			// map storing the error codes and their respective error_pages
		size_t									_client_max_body_size;	// lmax size (in bytes) of the client body according to config_file
		std::vector<std::string>				_index;					// index list according to config_file
		bool									_autoindex;				// autoindex as set by the config_file
		std::string								_root;					// root of the files as provided by the config_file
		std::vector<int>						_http_method;			// allowed http_Method as per config_file
		std::vector<Location>					_locations;				// vector of locaitons block as specified by the config_file
};

#endif