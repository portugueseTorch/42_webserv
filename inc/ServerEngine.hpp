#ifndef SERVERENGINE_HPP
# define SERVERENGINE_HPP

# include "Webserv.hpp"

class Server;

class ServerEngine {
	public:
		ServerEngine(std::list<Node> nodes);
		~ServerEngine();

		int configureServers();
		int setupServer(std::list<Node>::iterator &it);
		void displayServers();

		static std::vector<std::string> directives;

	private:
		std::list<Node> _nodes;			// list of nodes passed by the Parser
		std::vector<Server> _servers;	// vector of servers specified in config_file
		int _num_servers;				// number of servers setup in config_file

		void handleInvalidInput(std::list<Node>::iterator &it);
		int handleName(Server &new_server, std::list<Node>::iterator &it);
};

#endif
