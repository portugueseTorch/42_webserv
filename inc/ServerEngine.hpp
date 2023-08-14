#ifndef SERVERENGINE_HPP
# define SERVERENGINE_HPP

# include "Webserv.hpp"

enum Methods {
	GET,
	POST,
	HEAD,
	PUT,
	DELETE,
};

class Server;

class ServerEngine {
	public:
		ServerEngine(std::list<Node> nodes);
		~ServerEngine();

		// Take the information from the parser and build the individual servers, adding them to the _servers vector
		int configureServers();
		int configureServer(std::list<Node>::iterator &it);

		// Boots the individual servers by iterating on the _servers vector and calling server.bootServer()
		int setupServers();

		void displayServers();

		static std::vector<std::string> directives;

	private:
		std::list<Node> _nodes;			// list of nodes passed by the Parser
		std::vector<Server> _servers;	// vector of servers specified in config_file
		int _num_servers;				// number of servers setup in config_file

		void handleInvalidInput(std::list<Node>::iterator &it);
};

#endif
