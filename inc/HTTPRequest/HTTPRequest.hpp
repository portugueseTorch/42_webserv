#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include "Webserv.hpp"
#include "HTTPRequest/Parser.hpp"

class HTTPParser;

//TO-DO
//	improve protocol parsing
class HTTPRequest {
	public:
		HTTPRequest();
		void process(std::string request);
		~HTTPRequest();
		void displayParsedRequest();

		// Setters
		void			setPort(uint32_t port);
		void			setIPAddress(in_addr_t ip_address);

		// Getters
		std::string		getServerName();
		std::string		getMethod() const { return _method; }
		std::string		getRequestURI() const { return _requestURI; }
		std::string		getProtocol() const { return _protocol; }
		bool			getKeepAlive() const { return _keepAlive; }
		int				getContentLength() const { return _contentLength; }
		std::string		getBody() const { return _body; }
		uint32_t		getPort() const { return _port; }
		in_addr_t		getIPAddress() const { return _ip_address; }
		bool			success() const;
		int				getStatusCode() const { return _statusCode; }
		bool			isCGI;

		std::vector<std::string>			&getQueryParams() { return _query; }
		std::map<std::string, std::string>	&getAllParams() { return _params; }

		class invalidHTTPRequest : public std::exception {
			public:
				virtual const char* what() const throw() {
					return "could not create object";
					}
		};
		class parserNotInitialized : public std::exception {
			public:
				virtual const char* what() const throw() {
					return "parser not initialized";
					}
		};

	private:
		void parse();
		void setup();

		void displayRequest();

		void extractQuery(std::string URI);
		void checkIfConnection(std::string paramName, std::string paramContent);
		void checkContentLength(std::string paramName, std::string paramContent);

		HTTPParser							*parser;
		std::string							_content;
		int									_statusCode;

		std::string 						_method;
		std::string							_requestURI;
		std::string 						_protocol;
		std::vector<std::string>			_query;
		bool								_keepAlive;
		int									_contentLength;
		std::string							_body;
		uint32_t							_port;
		in_addr_t							_ip_address;
		std::string 						_server_name;

		std::map<std::string, std::string>	_params;
		bool								_parserCreated;
};

#endif