#pragma once
#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "Webserv.hpp"

// class Request
// {
// 	public:
// 		Request();
// 		~Request();

// 		// Getter
// 		uint32_t	getPort() const { return _port; }
// 		in_addr_t	getIPAddress() const { return _ip_address; }
// 		std::string	getServerName() const { return _server_name; }
// 		std::string	getHTTPProtocol() const { return _http_protocol; }
// 		std::string	getURI() const { return _uri; }
// 		bool		getIsCGI() const { return _is_CGI; }
// 		int			getHTTPMethod() const { return _http_method; }

// 		// Setter
// 		void		setCGI(bool is_CGI);
// 		void		setHTTPMethod(int http_method);
// 		void		setHTTPProtocol(std::string protocol);
// 		void		setURI(std::string uri);

// 	private:
// 		uint32_t	_port;				// this comes from the server block that is handling
// 		in_addr_t	_ip_address;		// this comes from the server that first establishes the connection
// 		std::string	_http_protocol;
// 		std::string _server_name;		// this comes from the HTTP request
// 		std::string	_uri;				// URI of the request
// 		int			_http_method;		// this comes from the HTTP request

// 		bool		_is_CGI;			// flag to signal whether or not it is a CGI request
// 		std::string	_cgi_path;			// full CGI path to be passed to execve()
// };

#endif
