#pragma once
#ifndef SERVERENGINE_HPP
# define SERVERENGINE_HPP

# include "Client.hpp"
# include "Webserv.hpp"

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

		// Runs the servers in a loop
		int		setupEpoll();
		int		acceptNewConnection(Server &owner_server);
		int 	readHTTPRequest(Client &client);
		bool	isServer(int fd);
		bool	isClient(int fd);
		int		runServers();
		int		modifyEpoll(int fd, int operation, int flags);
		int		closeConnection(int fd);

		int		sendResponse(Client &client);

		void	displayServers();

		static std::vector<std::string> directives;

	private:
		int						_epoll_fd;		// epoll file descriptor
		struct epoll_event		*_events;		// store returned events by epoll_wait()										

		std::list<Node>			_nodes;			// list of nodes passed by the Parser
		std::vector<Server>		_servers;		// vector of servers specified in config_file
		int						_num_servers;	// number of servers setup in config_file

		std::map<int,Server>	_server_map;	// map holding as key the server's fd, and the server as value
		std::map<int,Client>	_client_map;	// map holding as key the client's fd, and the client as value

		// Functions
		void	handleInvalidInput(std::list<Node>::iterator &it);

		int		sendErrResponse(Client &client);
		int 	sendRegResponse(Client &client);
		int 	sendCGIResponse(Client &client);
};

#endif
