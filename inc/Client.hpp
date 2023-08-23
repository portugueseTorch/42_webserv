#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Webserv.hpp"

class Client
{
	public:
		Client();
		~Client();

		// Getters
		int					getClientFD() const { return _client_fd; }
		// Setters
		void				setClientFD(int client_fd);
		void				setRequest(std::string request);

		int					setupClient();

	private:
		int					_client_fd;			// file descriptor of the client socket
		std::string			_request;			// request received from the client server

};

#endif
