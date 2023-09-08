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

Location::Location() {}

Location::~Location() {}

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
	_location = location;
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
	for (; it->_type == Parameter; it++)
		_index.push_back(it->_content);
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
	}
	_root = stash.back();
	return 0;
}
