#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "Webserv.hpp"

class Request
{
	public:
		Request();
		~Request();
	private:
		uint32_t	_port;			// this comes from the server block that is handling
		in_addr_t	_ip_address;	// this comes from the server that first establishes the connection
		std::string _server_name;	// this comes from the HTTP request
		Methods		_http_method;	// this comes from the HTTP request
};

#endif
