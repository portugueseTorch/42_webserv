#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include "Webserv.hpp"

class HTTPRequest {
	public:
		HTTPRequest();
		void process(std::string request);
		~HTTPRequest();
		void displayParsedRequest();

		// Setters
		void			setPort(uint32_t port);
		void			setIPAddress(in_addr_t ip_address);
		void			setURI(std::string uri);

		// Getters
		std::string		getServerName();
		std::string		getMethod() const { return _method; }
		std::string		getRequestURI() const { return _requestURI; }
		std::string		getProtocol() const { return _protocol; }
		bool			getKeepAlive() const { return _keepAlive; }
		size_t			getContentLength() const { return _contentLength; }
		std::string		getBody() const { return _body; }
		uint32_t		getPort() const { return _port; }
		in_addr_t		getIPAddress() const { return _ip_address; }
		bool			success() const;
		int				getStatusCode() const { return _statusCode; }
		bool			isCGI;
		bool			fullyParsed;

		std::vector<std::string>			&getQueryParams() { return _query; }
		std::map<std::string, std::string>	&getAllParams() { return _params; }

	private:

		void displayRequest();

		void extractQuery(std::string URI);
		void checkIfConnection(std::string paramName, std::string paramContent);
		
		bool alreadyExists(std::string param);
		bool processHeaderLine(std::string headerLine);
		int getLineType(std::string headerLine);

		bool checkRequestLine(std::string & headerLine);
		bool validMethod(std::string & headerLine);
		bool validRequestURI(std::string & headerLine);
		bool validAbsoluteURI(std::string req);
		void cleanUpURI(std::string req);
		bool validHTTPVersion(std::string & headerLine);

		bool checkContentLength(std::string headerLine);
		bool checkConnection(std::string headerLine);
		bool checkEncoding(std::string headerLine);

		bool addParam(std::string headerLine);
		void processChunked();
		bool headersSet();

		std::string							_content;
		int									_statusCode;

		std::string 						_method;
		std::string							_requestURI;
		std::string 						_protocol;
		std::vector<std::string>			_query;
		bool								_keepAlive;
		size_t								_contentLength;
		std::string							_body;
		uint32_t							_port;
		in_addr_t							_ip_address;
		std::string 						_server_name;

		std::map<std::string, std::string>	_params;
		bool								_emptyLine;
		bool								_chunked;
		bool								_finalChunk;
		int									_chunkSize;
		std::string							_chunkBuf;
};

#endif