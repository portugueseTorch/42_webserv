#include "ServerEngine/Location.hpp"

std::string	Location::_possibleDirectives[] = {
	"root",
	"index",
	"error_page",
	"fastcgi_pass",
	"autoindex",
	"http_method",
};

std::vector<std::string> Location::directives(Location::_possibleDirectives, \
	Location::_possibleDirectives + sizeof(Location::_possibleDirectives) / sizeof(std::string));

Location::Location() {
	std::string method_array[] = { "GET", "HEAD", "POST", "DELETE" };
	std::vector<std::string> http_method(method_array, method_array + sizeof(method_array) / sizeof(std::string));
	_http_method = http_method;
	_body_size_specified = false;
	_client_max_body_size = 1000000;
	_root = "";
	_has_return = false;
	_autoindex = false;
	_is_cgi = false;
}

Location::~Location() {}

void Location::displayLocationBlock() {
	std::cout << "\t  - Location " << getLocation() << std::endl;
	std::cout << "\t\tRoot: " << getRoot() << "\n";

	{
		std::cout << "\t\tIndex: ";
		std::vector<std::string> index = getIndex();
		for (std::vector<std::string>::iterator j = index.begin(); j != index.end(); j++)
			std::cout << *j << " ";
		std::cout << std::endl;
	}

	{
		std::cout << "\t\tError Pages:\n";
		std::map<int,std::vector<std::string> > error_pages = getErrorPages();
		for (std::map<int,std::vector<std::string> >::iterator j = error_pages.begin(); j != error_pages.end(); j++) {
			std::cout << "\t\t  - [ " << (*j).first << ", ";
			for (std::vector<std::string>::iterator k = (*j).second.begin(); k != (*j).second.end(); k++)
				std::cout << *k << " ";
			std::cout << "]\n";
		}
	}
	{
		std::cout << "\t\tHTTP Methods: ";
		std::vector<std::string> http_Method = getHTTPMethod();
		for (std::vector<std::string>::iterator i = http_Method.begin(); i != http_Method.end(); i++)
			std::cout << *i << " ";
		std::cout << std::endl;
	}
	{
		if (_body_size_specified)
			std::cout << "\t\tClient max body size: " << getClientMaxBodySize() << std::endl;
	}
	std::cout << "\t\tAuntoindex: " << getAutoindex() << std::endl;
	std::cout << "\t\tReturn: " << getReturn().first << " " << getReturn().second << std::endl;
	std::cout << "\t\tIs CGI: " << getIsCGI() << std::endl;
}

/*************************************/
/************** SETTERS **************/
/*************************************/

/**
 * @brief Sets the location LOCATION for the location block
 * 
 * @param location location to be set
 * @return 0
 */
int Location::setLocation(std::string location) {
	std::string file_extension = get_file_extension(location);
	if (location[0] != '/' && file_extension == "null") {
		log(std::cerr, ERROR, "Invalid location directive", location);
		return 1;
	}

	if (location != "/" && location[location.length() - 1] == '/')
		_location = location.substr(0, location.length() - 1);
	else
		_location = location;

	if (file_extension != "null") _is_cgi = true;
	return 0;
}

/**
 * @brief Sets the error pages for the location block by iterating over
 * the list of nodes IT, storing the result in [_error_pages], a map
 * where the key is an integer representing the error code, and the
 * value is a vector of strings representing the respective pages
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Location::setErrorPages(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check valid number of parameters
	if (stash.size() < 2) {
		log(std::cerr, ERROR, "Too few arguments for", "error_page");
		return 1;
	}

	for (size_t i = 0; i < stash.size() - 1; i++) {
		// Check all digits
		for (size_t j = 0; j < stash[i].length(); j++) {
			if (!isdigit(stash[i][j])) {
				log(std::cerr, ERROR, "Invalid error code", stash[i]);
				return 1;
			}
		}

		// Convert to int and check if it's between 400 and 599
		int code = std::atoi(stash[i].c_str());
		if (code < 400 || code > 599) {
			log(std::cerr, ERROR, "Invalid error code", stash[i]);
			return 1;
		}

		// Check if the code already exists in the map, and if so check if the current
		// error_page is already there to avoid duplicates
		if (_error_pages.find(code) != _error_pages.end()) {
			if (std::find(_error_pages[code].begin(), _error_pages[code].end(), stash.back()) != _error_pages[code].end())
				continue;
		}
		_error_pages[code].push_back(stash.back());
	}
	return 0;
}

/**
 * @brief Sets the HTTP Method allowed for the location block by iterating
 * over IT, storing the result in [_http_method] as a vector of strings
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Location::setHTTPMethod(std::list<Node>::iterator &it) {
	// Clear default allowed methods
	_http_method.clear();

	for (; it->_type == Parameter; it++) {
		if (it->_content == "GET")
			_http_method.push_back("GET");
		else if (it->_content == "HEAD")
			_http_method.push_back("HEAD");
		else if (it->_content == "POST")
			_http_method.push_back("POST");
		else if (it->_content == "DELETE")
			_http_method.push_back("DELETE");
		else {
			log(std::cerr, ERROR, "Invalid argument for http_method", it->_content);
			_http_method.clear();
			return 1;
		}
	}
	it--;

	// Check valid number of parameters
	if (_http_method.empty()) {
		log(std::cerr, ERROR, "Too few arguments for", "http_method");
		return 1;
	}
	return 0;
}


/**
 * @brief Sets the client_max_body_size by iterating over IT, storing 
 * it as a size_t in [_client_max_body_size]. The functions handles
 * numbers, as well as the k, m, and g abbreviations (case-insensitive)
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Location::setClientMaxBodySize(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check there is only 1 argument specified for client_max_body_size
	if (stash.size() != 1) {
		log(std::cerr, ERROR, "Invalid number of arguments for", "client_max_body_size");
		return 1;
	}

	// Check the argument is valid
	enum type { NUMBER, KILO, MEGA, GIGA, ERR };
	int flag = NUMBER;
	std::string body_size = stash.back();
	stash.clear();
	for (size_t i = 0; i < body_size.length(); i++) {
		if (body_size[i] == 'k' || body_size[i] == 'K' || \
			body_size[i] == 'm' || body_size[i] == 'M' || \
			body_size[i] == 'g' || body_size[i] == 'G') {
			if (i != (body_size.length() - 1)) {
				flag = ERR;
				break;
			}
			if (body_size[i] == 'k' || body_size[i] == 'K')
				flag = KILO;
			else if (body_size[i] == 'm' || body_size[i] == 'M')
				flag = MEGA;
			else
				flag = GIGA;
		} else if (!isdigit(body_size[i])) {
			flag = ERR;
			break;
		}
	}

	if (flag == ERR) {
		log(std::cerr, ERROR, "Invalid client_max_body_size", body_size);
		return 1;
	}

	// Adjust max body size according to the relevant flag
	size_t cmbs = (size_t) std::atoi(body_size.c_str());
	if (flag == KILO)
		cmbs *= 1000;
	else if (flag == MEGA)
		cmbs *= 1000000;
	else if (flag == GIGA)
		cmbs *= 1000000000;

	_client_max_body_size = cmbs;
	_body_size_specified = true;
	return 0;
}

/**
 * @brief Sets the return directive for the Location block by iterating over IT,
 * storing the result in the [_return] pair
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Location::setReturn(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check there are either 1 or 2 arguments
	if (stash.size() != 1 && stash.size() != 2) {
		log(std::cerr, ERROR, "Invalid number of arguments for", "return");
		return 1;
	}

	// Check the first argument is a valid status code
	std::string first = stash.at(0);
	for (size_t i = 0; i < first.length(); i++) {
		if (!isdigit(first[i])) {
			log(std::cerr, ERROR, "Invalid 'return' status code", first);
			return 1;
		}
	}

	int status_code = std::atoi(first.c_str());
	if (!ServerEngine::isSupportedStatusCode(status_code)) {
		log(std::cerr, ERROR, "Invalid 'return' status code", first);
		return 1;
	}

	// If status_code is in range 3xx, redirection URL is needed
	if (status_code >= 300 && status_code <= 305) {
		if (stash.size() == 2) {
			_return.first = status_code;
			_return.second = stash.at(1);
		} else {
			log(std::cerr, ERROR, "Missing 'return' URL for status code", first);
			return 1;
		}
	} else {
		_return.first = status_code;
		_return.second = stash.size() == 2 ? stash.at(1) : "";
	}

	_has_return = true;
	return 0;
}

/**
 * @brief Sets the indexes for the location block by iterating over IT, storing
 * the result in [_index] as a vector of strings
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Location::setIndex(std::list<Node>::iterator &it) {
	for (; it->_type == Parameter; it++) {
		if (it->_content.find('/') != std::string::npos) {
			log(std::cerr, ERROR, "Invalid index directive", it->_content);
			return 1;
		}
		_index.push_back(it->_content);
	}
	it--;

	// Check there is only 1 argument specified for client_max_body_size
	if (_index.size() < 1) {
		log(std::cerr, ERROR, "Invalid number of arguments for", "index");
		_index.clear();
		return 1;
	}

	return 0;
}

/**
 * @brief Sets the autoindex directive by iterating over IT, storing the
 * result as a boolean in [_autoindex]
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Location::setAutoindex(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check valid number of parameters
	if (stash.size() != 1) {
		log(std::cerr, ERROR, "Too few arguments for", "autoindex");
		return 1;
	}

	if (stash.back() == "on") {
		_autoindex = true;
		return 0;
	} else if (stash.back() == "off") {
		_autoindex = false;
		return 0;
	}

	log(std::cerr, ERROR, "Invalid autoindex parameter", it->_content);
	return 1;
}

/**
 * @brief Sets the root for the location block by iterating over IT, storing
 * the result in [_root] as a string
 * 
 * @note IT GETS ITERATED WITHIN THIS FUNCTION
 * 
 * @param it Reference to an iterator for the list of nodes built by the parser
 * @return Returns 0 on success, 1 if any invalid parameter is found
 */
int Location::setRoot(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check there is only 1 argument specified for client_max_body_size
	if (stash.size() != 1) {
		log(std::cerr, ERROR, "Invalid number of arguments for", "root");
		return 1;
	} else if (stash.back()[0] != '/') {
		log(std::cerr, ERROR, "Invalid root directive: must start with '/'", stash.back());
		return 1;
	}

	std::string root = stash.back();
	if (root != "/" && root[root.length()] == '/')
		_root = root.substr(0, root.length() - 1);
	else
		_root = root;
	return 0;
}
