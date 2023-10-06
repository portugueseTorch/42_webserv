#include "../inc/HTTPResponse.hpp"

HTTPResponse::HTTPResponse(HTTPRequest *request, Server *parent_server) {
	this->request = request;
	this->parent_server = parent_server;
	this->location_block = NULL;
	_response = "";
	_protocol = "";
	_time = "";
	_server = "";
	_content_type = "";
	_body = "";
	_body_length = 0;
	_header_length = 0;
	_response_length = 0;
	_status_code = request->success() ? 200 : request->getStatusCode();
	_content_type = "text/plain";
}

HTTPResponse::~HTTPResponse() {}

/////////////////////////////////////////////////////////////

void HTTPResponse::assignLocationBlock() {
	std::string uri = request->getRequestURI();
	std::vector<Location> &locations = parent_server->getLocations();
	std::cout << "Requested URI: " << uri << std::endl;

	// If the parent server has no location blocks, return
	if (locations.empty()) return;

	for (std::vector<Location>::iterator it = locations.begin(); it != locations.end(); it++) {
		int loc_length = it->getLocation().length();
		if (it->getLocation() == "/") {
			log(std::cout, SUCCESS, "Successfully assigned location block to uri", uri);
			location_block = &(*it);
			location_block->displayLocationBlock();
			return ;
		} else if (it->getLocation() == uri.substr(0, loc_length) && (uri[loc_length] == '\0' || uri[loc_length] == '/')) {
			log(std::cout, SUCCESS, "Successfully assigned location block to uri", uri);
			location_block = &(*it);
			location_block->displayLocationBlock();
			request->setURI(uri.substr(loc_length));
			return ;
		}
	}
	log(std::cout, INFO, "No location block assigned to uri", uri);
}

std::string HTTPResponse::getTime() {
	std::time_t now = std::time(NULL);
	std::tm* timeinfo = std::localtime(&now);
	const char* date = "%a, %d %b %Y %H:%M:%S %Z";
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), date, timeinfo);
	return std::string(buffer);
}

std::string formatTime(const struct tm* timeinfo) {
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z", timeinfo);
	return std::string(buffer);
}

std::string HTTPResponse::getContentType(std::string uri) {
	// Get index of last '.' of uri
	int lp = uri.find_last_of('.');
	if (uri.empty() || uri[uri.length() - 1] == '/' || lp == -1)
		return "directory";

	// Check the file extension
	std::string ex = uri.substr(lp);	
	if (ex == ".html")
		return "text/html";
	else if (ex == ".css" || uri.find("text/css") == 0)
		return "text/css";
	else if (ex == ".js")
		return "application/javascript";
	else if (ex == ".csv")
		return "text/csv";
	else if (ex == ".xml")
		return "text/xml";
	else if (ex == ".txt")
		return "text/plain";
	else if (ex == ".jpeg" || ex == ".jpg")
		return "image/jpeg";
	else if (ex == ".png")
		return "image/png";
	else if (ex == ".ico")
		return "image/ico";
	else if (ex == ".svg")
		return "image/svg+xml";
	else if (ex == ".py")
		return "text/html";
	else
		return "undefined";
}

std::string HTTPResponse::statusCodeToMessage() {
	switch (_status_code)
	{
		case 200:
			return "OK";
		case 201:
			return "Created";
		case 202:
			return "Accepted";
		case 204:
			return "No Content";

		case 400:
			return "Bad Request";
		case 403:
			return "Forbidden";
		case 404:
			return "Not Found";
		case 405:
			return "Method Not Allowed";
		case 406:
			return "Not Acceptable";
		case 410:
			return "Gone";
		case 411:
			return "Length Required";
		case 413:
			return "Payload Too Large";
		case 414:
			return "URI Too Long";
		case 415:
			return "Unsupported Media Type";

		case 500:
			return "Internal Server Error";
		case 501:
			return "Not Implemented";
		case 502:
			return "Bad Gateway";
		case 504:
			return "Gateway Timeout";
		case 505:
			return "HTTP Version Not Supported";
		default:
			return "";
	}
}

std::string HTTPResponse::getRelevantRoot() {
	if (location_block) {
		if (location_block->getRoot() != "")
			return "." + location_block->getRoot();
	}
	if (parent_server) {
		if (parent_server->getRoot() != "")
			return "." + parent_server->getRoot();
	}
	return "." + std::string(DEFAULT_ROOT);
}

void HTTPResponse::readFile(std::string file_path, struct stat *s) {
	std::ifstream file;
	int	file_size = s->st_size;
	file.open(file_path.c_str(), std::ios::binary);
	if (!file.is_open()) {
		log(std::cerr, ERROR, "Unable to open file", file_path);
		_status_code = 500;
		return ;
	}

	// Resize the string _body to fit the whole file, and read the content to the body
	if (request->getMethod() == "GET") {
		if (_body != "")
			_body.clear();
		_body.resize(file_size);
		file.read(const_cast<char *>(_body.data()), file_size);
		_body_length = file_size + 2;
		_last_modified = formatTime(std::localtime(&s->st_mtime));
	}
	file.close();
}

int HTTPResponse::readContent(std::string file_path) {
	// Set the content type
	_content_type = getContentType(file_path);
	std::cout << "Trying to read file " << file_path << std::endl;

	// Check if the file exists and we can open it
	struct stat s;
	if (stat(file_path.c_str(), &s) != 0) {
		log(std::cerr, ERROR, "Requested file does not exist", file_path);
		_status_code = 404;
		return 1;
	}
	if (access(file_path.c_str(), F_OK) != 0) {
		log(std::cerr, ERROR, "Requested file does not have necessary permissions", file_path);
		_status_code = 403;
		return 1;
	}

	// Open the file and read its contents
	readFile(file_path, &s);
	return 0;
}

void HTTPResponse::searchContent() {
	if (isError())
		return ;
	std::string uri = request->getRequestURI();
	std::string root = getRelevantRoot();
	bool		found = false;

	// If we have a directory, the file path will need to first check for the index file
	if (getContentType(uri) == "undefined") {
		_status_code = 415;
		return ;
	} else if (getContentType(uri) == "directory") {
		if (location_block) {
			std::vector<std::string> &indexes = location_block->getIndex();
			if (!indexes.empty()) {
				// Iterate over all possible index files
				for (std::vector<std::string>::iterator it = indexes.begin(); it != indexes.end(); it++) {
					_file_path = root + uri + "/" + *it;
					std::cout << "Trying to assign in location block " << _file_path << std::endl;
					if (file_is_valid(_file_path, F_OK)) {
						log(std::cout, SUCCESS, "File exists in location block", _file_path);
						found = true;
						break;
					}
				}
			}
			if (!found) {
				log(std::cerr, ERROR, "Invalid fallback files specified in 'index'", "");
				_status_code = 500;
				return ;
			}
		}
		if (parent_server && !found) {
			std::vector<std::string> &indexes = parent_server->getIndex();
			if (!indexes.empty()) {
				// Iterate over all possible index files
				for (std::vector<std::string>::iterator it = indexes.begin(); it != indexes.end(); it++) {
					_file_path = root + uri + "/" + *it;
					std::cout << "Trying to assign in server block " << _file_path << std::endl;
					if (file_is_valid(_file_path, F_OK)) {
						log(std::cout, SUCCESS, "File exists in parent server", _file_path);
						found = true;
						break;
					}
				}
			}
			if (!found) {
				log(std::cerr, ERROR, "Invalid fallback files specified in 'index'", "");
				_status_code = 500;
				return ;
			}
		}
	} else {
		_file_path = root + uri;
	}

	// Attempt to read the contents of the file
	if (!readContent(_file_path))
		_status_code = 200;
}

void HTTPResponse::searchErrorContent() {
	std::string root = getRelevantRoot();
	std::string error_file_path;
	bool found = false;

	if (location_block && location_block->getErrorPages().count(_status_code)) {
		std::vector<std::string> &error_pages = location_block->getErrorPages()[_status_code];
		// Iterate over all possible error_pages
		for (std::vector<std::string>::iterator it = error_pages.begin(); it != error_pages.end(); it++) {
			error_file_path = root + "/" + *it;
			std::cout << "Trying to assign error_page in location block " << error_file_path << std::endl;
			if (file_is_valid(error_file_path, F_OK)) {
				log(std::cout, SUCCESS, "File exists in location block", error_file_path);
				found = true;
				break;
			}
		}
	}
	if (parent_server && parent_server->getErrorPages().count(_status_code) && !found) {
		std::vector<std::string> &error_pages = parent_server->getErrorPages()[_status_code];
		// Iterate over all possible error_pages
		for (std::vector<std::string>::iterator it = error_pages.begin(); it != error_pages.end(); it++) {
			error_file_path = root + "/" + *it;
			std::cout << "Trying to assign error_page in server block " << error_file_path << std::endl;
			if (file_is_valid(error_file_path, F_OK)) {
				log(std::cout, SUCCESS, "File exists in parent server", error_file_path);
				found = true;
				break;
			}
		}
	}
	if (!found) {
		error_file_path = DEFAULT_ERROR_FILE;
	}

	if (readContent(error_file_path) && error_file_path == DEFAULT_ERROR_FILE) {
		_body.clear();
		_body = "";
		_body_length = 0;
		_status_code = 500;
		log(std::cerr, WARNING, "Specified default error file does not exist", error_file_path);
	}
}

bool HTTPResponse::validClientBodySize() {
	size_t max_body_size;

	if (location_block && location_block->clientBodySizeSet())
		max_body_size = location_block->getClientMaxBodySize();
	else if (parent_server)
		max_body_size = parent_server->getClientMaxBodySize();
	else
		max_body_size = 1000000;

	return request->getBody().size() <= max_body_size;
}

bool HTTPResponse::isAllowedMethod() {
	std::vector<std::string> allowed_methods;

	if (location_block)
		allowed_methods = location_block->getHTTPMethod();
	else if (parent_server)
		allowed_methods = parent_server->getHTTPMethod();
	for (std::vector<std::string>::iterator it = allowed_methods.begin(); it != allowed_methods.end(); it++) {
		if (request->getMethod() == *it) {
			log(std::cout, INFO, "Method", *it);
			return true;
		}
	}
	return false;
}

int	HTTPResponse::build() {
	int res = 0;
	// Assign the location block for the response
	std::cout << "aqui estamos " << _status_code << std::endl;
	this->assignLocationBlock();

	// General Headers
	_protocol = request->getProtocol();
	_time = getTime();
	_server = "Webserv/42.0";

	// Content Headers
	if (!validClientBodySize())
		_status_code = 413;
	if (!isAllowedMethod())
		_status_code = 405;
	// if (request->getMethod() == "GET")
	searchContent();
	if (isError())
		searchErrorContent();

	std::stringstream ss;
	ss << _status_code;
	_response += _protocol + " "  + ss.str() + " " + statusCodeToMessage() + "\r\n";
	ss.str("");
	ss.clear();

	_response += "Date: " + _time + "\r\n";
	_response += "Server: " + _server + "\r\n";
	// _response += "Last-Modified: " + _last_modified + "\r\n";

	if (!request->isCGI || isError()) {
		ss << _body_length;
		_response += "Content-Length: " + ss.str() + "\r\n";
		ss.str("");
		ss.clear();
	}

	_response += "Content-Type: " + _content_type + "\r\n";
	if (!request->isCGI || isError())
		_response += "\r\n";

	_header_length = _response.length();
	if (request->getMethod() == "HEAD")
		_body_length = 0;
	if (request->isCGI && !isError()) {
		res = buildCGIResponse();
	} else if (_body != "" && request->getMethod() != "HEAD")
		_response += _body + "\r\n";

	_response_length = _header_length + _body_length;

	// std::cout << "Total length of the response is: " << _response_length << " and " << _response.length() << std::endl;
	// std::cout << _response;
	return res;
} 

int HTTPResponse::buildCGIResponse() {
	int pipe_fd[2];
	int pid;

	_response.clear();
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

		// char *args[] = { (char *)"/usr/bin/python3", strdup(_file_path.c_str()), NULL };
		char *args[] = { (char *)"/usr/bin/python3", (char *)"cgi-bin/main.py", NULL };
		std::vector<std::string> queryAndBody = request->getQueryParams();
		std::string uri = request->getRequestURI();
		uri = uri[0] == '/' ? uri.substr(1) : uri;
		std::string webservPath = "webservPath=" + uri;
		std::string webservMethod = "webservMethod=" + request->getMethod();
		queryAndBody.push_back(webservPath);
		queryAndBody.push_back(webservMethod);

		std::string body = request->getBody();
		// log(std::cerr, INFO, "body", body);

		std::string contentType = request->getAllParams()["content-type"];
		std::string contentLength = request->getAllParams()["content-length"];
		if (contentType.size() && contentType.find("boundary=") != std::string::npos) {
			size_t sep;
			sep = contentType.find("boundary=");
			std::string boundary = contentType.substr(sep);
			body = "content=" + urlEncode(body) + "&";
			body.append(boundary + "&");
			body.append("content-length=" + contentLength);
		}
		std::stringstream ss(body);
		std::vector<std::string> splitBody;
		std::string buf;


		while (std::getline(ss, buf, '&')) {
			splitBody.push_back(buf);
		}
		queryAndBody.insert(queryAndBody.end(), splitBody.begin(), splitBody.end());
		
		unsigned char **envp = vectToArr(queryAndBody);
		// std::vector<std::string>::iterator it = queryAndBody.begin();
		// log(std::cerr, INFO, "Printing body", "");
		// for (; it != queryAndBody.end(); it++) {
		// 	std::cerr << *it << std::endl;
		// }
		execve("/usr/bin/python3", args, (char **)envp);
		delete []envp;
	} else {
		close(pipe_fd[1]);
		wait(NULL);
		unsigned char msg[MAX_LENGTH + 1] = "";
		if (_body != "")
			_body.clear();
		std::stringstream ss;

		int bytes = read(pipe_fd[0], msg, MAX_LENGTH);
		while (bytes != 0) {
			if (bytes == -1) {
				log(std::cerr, ERROR, "read() call failed", "");
				return 1;
			}
			std::string toAdd(reinterpret_cast<const char*>(msg), bytes);
			// std::cerr << toAdd << std::endl;
			_body += toAdd;
			bytes = read(pipe_fd[0], msg, MAX_LENGTH);
		}

		_header_length = _response.size();
		_body_length = _body.size() + 2;
		_response += _body + "\r\n";
		_response_length = _header_length + _body_length;
	}
	return 0;
}

std::string HTTPResponse::urlEncode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

   for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
		char c = *it;
        // Keep alphanumeric and other accepted characters intact
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << '%' << std::setw(2) << int((unsigned char)c);
    }

    return escaped.str();
}

unsigned char**	HTTPResponse::vectToArr(std::vector<std::string> vect) {
	unsigned char **envp = new unsigned char*[vect.size() + 1];
	size_t i = 0;
	for (; i < vect.size(); i++) {
		envp[i] = new unsigned char[vect[i].size() + 1];
		strcpy((char *)envp[i], vect[i].c_str());
	}
	envp[i] = NULL;
	return envp;
}