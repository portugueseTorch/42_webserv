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
	_cont_length = 0;
}

Client::~Client() {
	if (request)
		delete request;
}

void Client::reset() {
	_file_buff = "";
	_status_code = 0;
	_response = "";
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
		log(std::cerr, ERROR, "fcntl() call failed", "");
		return 1;
	}

	if (fcntl(_client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {	// add O_NONBLOCK to previous flags, and set them
		log(std::cerr, ERROR, "fcntl() call failed", "");
		return 1;
	}

	std::stringstream ss;
	ss << _client_fd;
	log(std::cout, INFO, "New Client set up", ss.str());
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
	request = new HTTPRequest;
	if (!parent_server) {
		log(std::cout, ERROR, "Somehow parent server is not assiged...", "");
		return 1;
	}
	request->setPort(parent_server->getPort());
	request->setIPAddress(parent_server->getIPAddress());
	request->process(request_str);
	if (!request->success()) {
		_status_code = 400;
		return 1;
	}
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
	else if (ex == ".css" || uri.find("text/css") == 0)
		return "text/css\r\n";
	else if (ex == ".js")
		return "application/javascript\r\n";
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
	else if (ex == ".ico")
		return "image/ico\r\n";
	else if (ex == ".svg")
		return "image/svg+xml\r\n";
	else if (ex == ".py")
		return "text/html\r\n";
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
	if (location_block && location_block->getRoot() != "") {
		if (location_block->getRoot() != "/") {
			// log(std::cerr, ERROR, "aqui", location_block->getRoot());
			// exit(0);
			log(std::cout, INFO, "root", location_block->getRoot());
			root = request->isCGI ? "./" : location_block->getRoot();
		}
	} else if (parent_server && parent_server->getRoot() != "") {
		if (parent_server->getRoot() != "/")
			root = parent_server->getRoot();
	} else
		root = "/default";	// TODO: Decide what the fallback folder is

	// If uri does not have '.', or the last character of uri is '/', it's a directory
	_file_type = getContentType(uri);
	if (_file_type == "undefined") {
		log(std::cerr, ERROR, "Media type not supported", uri);
		this->setStatusCode(415);
		return 0;
	} else if (uri.find("/cgi-bin") == 0) {
		root = "";
		_file_type = "text/html\r\n";
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
						in_file.open(tmp.c_str());
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
						in_file.open(tmp.c_str());
						if (!in_file.is_open())
							continue;
						_file_type = getContentType(*it);
						_uri = "/" + *it;
						uri = _uri;
						found = true;
						in_file.close();
					}
				}
			}
		}
		if (!found) {
			// TODO: Handle autoindex
			log(std::cerr, ERROR, "Requested URI does not exist", uri);
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

			in_file.open(full_path.c_str());
			// If we fail to open the file (knowing that it exists), set server error
			if (!in_file.is_open()) {
				log(std::cerr, ERROR, "Unable to open file", full_path);
				this->setStatusCode(500);
				return 1;
			}
			while (std::getline(in_file, buff)) {
				_file_buff += buff;
				if (!in_file.eof())
					_file_buff += "\n";
			}
			_cont_length = _file_buff.length();
			this->setStatusCode(200);	// TODO: Check nuances here
			in_file.close();
		} else {	// If the file exists but we don't have access to it
			log(std::cerr, ERROR, "Requested URI does not have necessary permissions", full_path);
			this->setStatusCode(403);
		}
	} else {	// If the resource does not exist
		log(std::cerr, ERROR, "Requested URI does not exist", full_path);
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
	_response += request->getProtocol() + " ";

	_uri = request->getRequestURI();
	// Otherwise if method is GET, read content of the requested file
	if (request->getMethod() == "GET") {
		searchRequestedContent(_uri);
	}

	// If the status_code is not set, something went wrong
	if (_status_code == 0) {
		this->setStatusCode(500);
		log(std::cerr, ERROR, "Something went wrong", "");
	}

	std::stringstream ss;
	ss << _status_code;
	// Set the first line of the Response
	_response += ss.str() + " " + statusCodeToMessage(_status_code);


	// Add Date
	std::time_t now = std::time(NULL);
	std::tm* timeinfo = std::localtime(&now);
	const char* date = "%a, %d %b %Y %H:%M:%S %Z\r\n";
	char buffer[80]; // Sufficient buffer size to hold the formatted string
	std::strftime(buffer, sizeof(buffer), date, timeinfo);
	_response += "Date: " + std::string(buffer);

	// Add Server info
	_response += "Server: Webserver/42.0\r\n";

	ss.str("");
	ss.clear(); // Clear state flags.
	// Add Content-Length
	ss << _cont_length;
	if (!request->isCGI)
		_response += "Content-Length: " + ss.str() += "\r\n";
	std::cout << "###################\n" << "Content-Length: " + ss.str() << "\n###################\n";

	// Add Content-Type
	_response += "Content-Type: " + _file_type;

	// Add Body
	if (request->isCGI) {
		return buildCGIResponse();
	} else {
		_response += "\r\n\r\n";
		_response += _file_buff + "\r\n";
	}

	std::cout << _response << std::endl;
	return 0;
}

/**
 * The function `buildCGIResponse` creates a pipe, forks a child process, and executes a Python script
 * to generate a CGI response, which is then read from the pipe and added to the response string.
 * 
 * @return an integer value.
 */
int	Client::buildCGIResponse() {
	int pipe_fd[2];
	int pid;

	// Create the pipe
	if (pipe(pipe_fd) == -1) {
		log(std::cerr, ERROR, "pipe() call failed", "");
		return 1;
	}
	// Fork and execve the script on the child pr
	if ((pid = fork()) == -1) {
		log(std::cerr, ERROR, "fork() call failed", "");
		return 1;
	}

	if (pid == 0) {
		dup2(pipe_fd[1], STDOUT_FILENO);
		close(pipe_fd[0]);
		char *args[] = { (char *)"/usr/bin/python3", (char *)"cgi-bin/cgi.py", NULL };
		char **envp = vectToArr(request->getQueryParams());
		execve("/usr/bin/python3", args, envp);
		//need error handling so request is not left pending
		delete []envp;
	} else {
		close(pipe_fd[1]);
		wait(NULL);
		char msg[MAX_LENGTH] = "";
		std::string body;
		std::stringstream ss;

		int bytes = read(pipe_fd[0], msg, MAX_LENGTH);
		while (bytes != 0) {
			if (bytes == -1) {
				log(std::cerr, ERROR, "read() call failed", "");
				return 1;
			}
			body += msg;
			bytes = read(pipe_fd[0], msg, MAX_LENGTH);
		}

		ss << body.size();
		_response += "Content-Length: " + ss.str() += "\r\n\r\n";
		_response += body + "\r\n";
	}
	return 0;
}

char**	Client::vectToArr(std::vector<std::string> vect) {
	char **envp = new char*[vect.size() + 1];
	size_t i = 0;
	for (; i < vect.size(); i++) {
		envp[i] = new char[vect[i].size() + 1];
		strcpy(envp[i], vect[i].c_str());
	}
	envp[i] = NULL;
	return envp;
}