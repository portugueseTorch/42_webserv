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
		int					setupClient();

	private:
		int					_client_fd;			// file descriptor of the client socket

};

#endif
