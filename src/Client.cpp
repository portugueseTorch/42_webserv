#include "../inc/Client.hpp"

int Client::num_clients = 0;

Client::Client() {
	num_clients++;
	_client_fd = -1;
	_response = "";
	parent_server = NULL;
	location_block = NULL;
	request = NULL;
	_client_id = num_clients;
	_file_buff = "";
	_status_code = 0;
}

Client::~Client() {
	if (request)
		delete request;
}

void Client::reset() {
	_file_buff = "";
	_status_code = 0;
	_response = "";
	parent_server = NULL;
	location_block = NULL;
	request = NULL;
}

void Client::setClientFD(int client_fd) {
	_client_fd = client_fd;
}

void Client::setRequest(std::string request_str) {
	_request_str = request_str;
}

void Client::setStatusCode(int status_code) {
	_status_code = status_code;
}

/**
 * @brief Sets _client_fd to be non-blocking
 * 
 * @return int Returns 0 on success, and 1 on failure
 */
int Client::setupClient() {
	// Set socket to be non-blocking
	int flags = fcntl(_client_fd, F_GETFL, 0);	// get current flags
	if (flags == -1) {
		log(std::cerr, MsgType::ERROR, "fcntl() call failed", "");
		return 1;
	}

	if (fcntl(_client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {	// add O_NONBLOCK to previous flags, and set them
		log(std::cerr, MsgType::ERROR, "fcntl() call failed", "");
		return 1;
	}
	log(std::cout, MsgType::INFO, "New Client set up", std::to_string(_client_fd));
	return 0;
}

/**
 * @brief Parses the HTTP request and builds the Request object
 * with the appropriat attributes according to the parsing
 * 
 * @param request String with the contents of the HTTP request
 * @return int Returns 0 on success, and 1 on failure
 */
int Client::parseHTTPRequest(std::string request_str) {
	if (request)
		delete request;
	request = new Request;
	_request_str = request_str;
	std::string first_line = request_str.substr(0, request_str.find_first_of('\n'));
	int first_space = first_line.find_first_of(' ') + 1;
	int last_space = first_line.find_last_of(' ');
	std::string uri = first_line.substr(first_space, last_space - first_space);
	if (uri[uri.length() - 1] == '/')
		uri = uri.substr(0, uri.length() - 1);
	request->setURI(uri);
	return 0;
}

/**
 * @brief Takes a status_code and converts it to
 * the appropriate status message to display in the Response.
 * The returned string includes the appropriate linebreak
 * 
 * @param status_code Status code to map to a message
 * @return std::string Message associated with status_code
 */
std::string Client::statusCodeToMessage(int status_code) {
	switch (status_code)
	{
		case 200:
			return "OK\r\n";
		case 201:
			return "Created\r\n";
		case 202:
			return "Accepted\r\n";
		case 204:
			return "No Content\r\n";

		case 400:
			return "Bad Request\r\n";
		case 403:
			return "Forbidden\r\n";
		case 404:
			return "Not Found\r\n";
		case 405:
			return "Method Not Allowed\r\n";
		case 406:
			return "Not Acceptable\r\n";
		case 410:
			return "Gone\r\n";
		case 411:
			return "Length Required\r\n";
		case 414:
			return "URI Too Long\r\n";
		case 415:
			return "Unsupported Media Type\r\n";

		case 500:
			return "Internal Server Error\r\n";
		case 501:
			return "Not Implemented\r\n";
		case 502:
			return "Bad Gateway\r\n";
		case 504:
			return "Gateway Timeout\r\n";
		case 505:
			return "HTTP Version Not Supported\r\n";
		default:
			return "";
	}
}


std::string Client::getContentType(std::string uri) {
	// Get index of last '.' of uri
	int lp = uri.find_last_of('.');
	if (uri[uri.length() - 1] == '/' || lp == -1)
		return "directory";

	// Check the file extension
	std::string ex = uri.substr(lp);	
	if (ex == ".html")
		return "text/html\r\n";
	else if (ex == ".css")
		return "text/css\r\n";
	else if (ex == ".csv")
		return "text/csv\r\n";
	else if (ex == ".xml")
		return "text/xml\r\n";
	else if (ex == ".txt")
		return "text/plain\r\n";
	// else if (ex == ".gif")
	// 	return "image/gif\r\n";
	else if (ex == ".jpeg" || ex == ".jpg")
		return "image/jpeg\r\n";
	else if (ex == ".png")
		return "image/png\r\n";
	else
		return "undefined";
}

/**
 * @brief searches for the content associated with the uri URI.
 * If the content exists and is accessible, the file is read and
 * stored in _file_buff, and the status code is set appropriately.
 * 
 * @param uri URI of the request associated with the client
 * @return int Returns 0 on success, and 1 on failure
 */
int Client::searchRequestedContent(std::string uri) {

	std::string root = "";

	// Check if there is a root specified or not
	if (location_block && location_block->getRoot() != "")
		root = location_block->getRoot();
	else if (parent_server && parent_server->getRoot() != "")
		root = parent_server->getRoot();
	else
		root = "/default";	// TODO: Decide what the fallback folder is

	// If uri does not have '.', or the last character of uri is '/', it's a directory
	_file_type = getContentType(uri);
	if (_file_type == "undefined") {
		log(std::cerr, MsgType::ERROR, "Media type not supported", uri);
		this->setStatusCode(415);
		return 0;
	} else if (_file_type == "directory") {
		// Check if an 'index' directive was provided
		bool found = false;
		if (location_block) {
			std::vector<std::string> &possible_indexes = location_block->getIndex();
			if (!possible_indexes.empty()) {
				for (std::vector<std::string>::iterator it = possible_indexes.begin(); it != possible_indexes.end(); it++) {
					struct stat sb;
					std::ifstream in_file;
					std::string tmp = "." + root + "/" + *it;
					std::cout << "\tTesting " << tmp << std::endl;
					if (stat(tmp.c_str(), &sb) == 0 && access(tmp.c_str(), R_OK) == 0) {
						in_file.open(tmp);
						if (!in_file.is_open())
							continue;
						_file_type = getContentType(*it);
						_uri = "/" + *it;
						uri = _uri;
						found = true;
					}
				}
			}
		}
		if (parent_server && !found) {
			std::vector<std::string> &possible_indexes = parent_server->getIndex();
			if (!possible_indexes.empty()) {
				for (std::vector<std::string>::iterator it = possible_indexes.begin(); it != possible_indexes.end(); it++) {
					struct stat sb;
					std::ifstream in_file;
					std::string tmp = "." + root + "/" + *it;
					std::cout << "\tTesting " << tmp << std::endl;
					if (stat(tmp.c_str(), &sb) == 0 && access(tmp.c_str(), R_OK) == 0) {
						in_file.open(tmp);
						if (!in_file.is_open())
							continue;
						_file_type = getContentType(*it);
						_uri = "/" + *it;
						uri = _uri;
						found = true;
					}
				}
			}
		}
		if (!found) {
			// TODO: Handle autoindex
			log(std::cerr, MsgType::ERROR, "Requested URI does not exist", uri);
			this->setStatusCode(403);
			return 0;
		}
	} else {
		// Check if the URI starts with '/', and if it doesn't add it
		if (uri[0] != '/' && root[root.length() - 1] != '/')
			uri = '/' + uri;
	}

	// Create the full path and test that we have access to it
	std::string full_path = '.' + root + uri;
	std::cout << full_path << std::endl;

	// Check if the resource exists
	struct stat sb;
	if (stat(full_path.c_str(), &sb) == 0) {
		if (access(full_path.c_str(), R_OK) == 0) {	// If we have access to it
			// Open the file and read it into a string
			std::ifstream in_file;
			std::string buff;

			in_file.open(full_path);
			// If we fail to open the file (knowing that it exists), set server error
			if (!in_file.is_open()) {
				log(std::cerr, MsgType::ERROR, "Unable to open file", full_path);
				this->setStatusCode(500);
				return 1;
			}
			while (std::getline(in_file, buff)) {
				_file_buff += buff;
				if (!in_file.eof())
					_file_buff += "\n";
			}
			std::cout << _file_buff << std::endl;
			this->setStatusCode(200);	// TODO: Check nuances here
			in_file.close();
		} else {	// If the file exists but we don't have access to it
			log(std::cerr, MsgType::ERROR, "Requested URI does not have necessary permissions", full_path);
			this->setStatusCode(403);
		}
	} else {	// If the resource does not exist
		log(std::cerr, MsgType::ERROR, "Requested URI does not exist", full_path);
		this->setStatusCode(404);
	}
	return 0;
}

/**
 * @brief Builds the HTTP response based off of the attributes
 * populated during parseHTTPRequest() function in the Request
 * object
 * 
 * @return int Returns 0 on success, and 1 on failure
 */
int Client::buildHTTPResponse() {
	// Get HTTP protocol from the request
	_response += request->getHTTPProtocol() + " ";


	// If the requested uri is CGI, call the appropriate function
	// TODO:

	_uri = request->getURI();
	// Otherwise if method is GET, read content of the requested file
	if (request->getHTTPMethod() == Method::GET) {
		searchRequestedContent(_uri);
	}

	// If the status_code is not set, something went wrong
	if (_status_code == 0) {
		this->setStatusCode(500);
		log(std::cerr, MsgType::ERROR, "Something went wrong", "");
	}

	// Set the first line of the Response
	_response += std::to_string(_status_code) + " " + statusCodeToMessage(_status_code);


	// Add Date
	std::time_t now = std::time(nullptr);
	std::tm* timeinfo = std::localtime(&now);
	const char* date = "%a, %d %b %Y %H:%M:%S %Z\r\n";
	char buffer[80]; // Sufficient buffer size to hold the formatted string
	std::strftime(buffer, sizeof(buffer), date, timeinfo);
	_response += "Date: " + std::string(buffer);

	// Add Server info
	_response += "Server: Webserver/42.0\r\n";

	// Add Content-Length
	int content_length = _file_buff.length();
	_response += "Content-Length: " + std::to_string(content_length) += "\r\n";

	// Add Content-Type
	_response += "Content-Type: " + _file_type + "\r\n";

	// Add Body
	_response += "\r\n" + _file_buff + "\r\n";

	std::cout << _response << std::endl;
	return 0;
}