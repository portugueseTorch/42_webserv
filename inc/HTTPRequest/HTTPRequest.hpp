#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include "Webserv.hpp"
#include "HTTPRequest/Parser.hpp"

class HTTPParser;

//TO-DO
//	do exceptions
//	improve accept parsing
class HTTPRequest {
	public:
		HTTPRequest(std::string request);
		~HTTPRequest();
		void displayParsedRequest();

		// Getters
		std::string		getMethod() const { return _method; }
		std::string		getRequestURI() const { return _requestURI; }
		std::string		getProtocol() const { return _protocol; }
		bool			getKeepAlive() const { return _keepAlive; }
		int				getContentLength() const { return _contentLength; }
		std::string		getBody() const { return _body; }

		std::map<std::string, std::string>	&getAcceptParams() { return _accept; }
		std::map<std::string, std::string>	&getQueryParams() { return _query; }
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
		HTTPRequest(){};

		void parse();
		void setup();
		void cleanExit();

		void displayRequest();

		void extractQuery(std::string URI);
		void checkIfConnection(std::string paramName, std::string paramContent);
		void checkIfAccept(std::string paramName, std::string paramContent);
		void checkContentLength(std::string paramName, std::string paramContent);

		HTTPParser * parser;
		std::string _content;

		std::string 						_method;
		std::string							_requestURI;
		std::string 						_protocol;
		std::map<std::string, std::string>	_accept;
		std::map<std::string, std::string>	_query;
		bool								_keepAlive;
		int									_contentLength;
		std::string							_body;

		std::map<std::string, std::string>	_params;
		bool								_parserCreated;


};

#endif