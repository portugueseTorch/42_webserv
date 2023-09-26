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
	if (uri[uri.length() - 1] == '/' || lp == -1)
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
	if (_body != "")
		_body.clear();
	_body.resize(file_size);
	file.read(const_cast<char *>(_body.data()), file_size);
	_body_length = file_size + 2;
	_last_modified = formatTime(std::localtime(&s->st_mtime));
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
	std::string uri = request->getRequestURI();
	std::string file_path;
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
					file_path = root + uri + "/" + *it;
					std::cout << "Trying to assign in location block " << file_path << std::endl;
					if (file_is_valid(file_path, F_OK)) {
						log(std::cout, SUCCESS, "File exists in location block", file_path);
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
					file_path = root + uri + "/" + *it;
					std::cout << "Trying to assign in server block " << file_path << std::endl;
					if (file_is_valid(file_path, F_OK)) {
						log(std::cout, SUCCESS, "File exists in parent server", file_path);
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
		file_path = root + uri;
	}

	// Attempt to read the contents of the file
	if (!readContent(file_path))
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

int	HTTPResponse::build() {
	// Assign the location block for the response
	this->assignLocationBlock();

	// General Headers
	_protocol = request->getProtocol();
	_time = getTime();
	_server = "Webserv/42.0";

	// Content Headers
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

	ss << _body_length;
	_response += "Content-Length: " + ss.str() + "\r\n";
	ss.str("");
	ss.clear();

	_response += "Content-Type: " + _content_type + "\r\n";
	_response += "\r\n";
	_header_length = _response.length();

	if (_body != "")
		_response += _body + "\r\n";

	_response_length = _header_length + _body_length;

	// std::cout << "Total length of the response is: " << _response_length << " and " << _response.length() << std::endl;
	return 0;
} 
