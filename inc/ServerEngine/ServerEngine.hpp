#pragma once
#ifndef SERVERENGINE_HPP
# define SERVERENGINE_HPP

# include "Client.hpp"
# include "Webserv.hpp"

enum Methods {
	GET,
	POST,
	HEAD,
	PUT,
	DELETE,
};

class Server;
class Node;
class Client;

class ServerEngine {
	public:
		ServerEngine(std::list<Node> nodes);
		~ServerEngine();

		// Take the information from the parser and build the individual servers, adding them to the _servers vector
		int		configureServers();
		int		configureServer(std::list<Node>::iterator &it);

		// Boots the individual servers by iterating on the _servers vector and calling server.bootServer()
		int		setupServers();
		int		assignServer(Client &client);
		void	modifySet(int fd, int set, int op);

		// Runs the servers in a loop
		int		setupSets();
		int		acceptNewConnection(Server &owner_server);
		int 	readHTTPRequest(Client &client);
		bool	isServer(int fd);
		bool	isClient(int fd);
		int		runServers();
		int		closeConnection(int fd);
		void	checkConnectionTimeouts();

		int		sendResponse(Client &client);

		void	displayServers();

		static std::vector<std::string> directives;
		static std::string possibleDirectives[];
		static std::vector<int> supported_status_codes;
		static bool isSupportedStatusCode(int);

	private:
		fd_set					_read_set;		// set of sockets being monitored for read events
		fd_set					_write_set;		// set of sockets being monitored for write events
		int						_max_fd;		// biggest fd being monitored, use for looping after select()

		std::list<Node>			_nodes;			// list of nodes passed by the Parser
		std::vector<Server>		_servers;		// vector of servers specified in config_file
		int						_num_servers;	// number of servers setup in config_file

		std::map<int,Server>	_server_map;	// map holding as key the server's fd, and the server as value
		std::map<int,Client>	_client_map;	// map holding as key the client's fd, and the client as value

		// Functions
		void	handleInvalidInput(std::list<Node>::iterator &it);
};

#endif
