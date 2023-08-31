#pragma once
#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "Webserv.hpp"

class Request
{
	public:
		Request();
		~Request();

		// Getter
		bool	getIsCGI() const { return _is_CGI; }
		bool	getIsError() const { return _is_error; }

		// Setter
		void	setCGI(bool is_CGI);
		void	setHTTPMethod(int http_method);

	private:
		uint32_t	_port;				// this comes from the server block that is handling
		in_addr_t	_ip_address;		// this comes from the server that first establishes the connection
		std::string _server_name;		// this comes from the HTTP request
		int			_http_method;		// this comes from the HTTP request
		int			_error_code;		// 

		bool		_is_CGI;			// flag to signal whether or not it is a CGI request
		std::string	_cgi_path;			// full CGI path to be passed to execve()
		bool		_is_error;			// flag to signal whether or not the request has an error();
};

#endif
