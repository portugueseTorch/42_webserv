#include "../inc/Location.hpp"

std::vector<std::string> Location::directives = {
	"root",
	"index",
	"error_page",
	"fastcgi_pass",
	"autoindex",
	"http_method",
};

Location::Location() {}

Location::~Location() {}

/*************************************/
/************** SETTERS **************/
/*************************************/

int Location::setErrorPages(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == NodeType::Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check valid number of parameters
	if (stash.size() < 2) {
		log(std::cerr, MsgType::ERROR, "Too few arguments for", "error_page");
		return 1;
	}

	for (int i = 0; i < stash.size() - 1; i++) {
		// Check all digits
		for (int j = 0; j < stash[i].length(); j++) {
			if (!isdigit(stash[i][j])) {
				log(std::cerr, MsgType::ERROR, "Invalid error code", stash[i]);
				return 1;
			}
		}

		// Convert to int and check if it's between 400 and 599
		int code = stoi(stash[i]);
		if (code < 400 || code > 599) {
			log(std::cerr, MsgType::ERROR, "Invalid error code", stash[i]);
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

int Location::setClientMaxBodySize(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == NodeType::Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check there is only 1 argument specified for client_max_body_size
	if (stash.size() != 1) {
		log(std::cerr, MsgType::ERROR, "Invalid number of arguments for", "client_max_body_size");
		return 1;
	}

	// Check the argument is valid
	enum type { NUMBER, KILO, MEGA, GIGA, ERR };
	int flag = NUMBER;
	std::string body_size = stash.back();
	stash.clear();
	for (int i = 0; i < body_size.length(); i++) {
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
		log(std::cerr, MsgType::ERROR, "Invalid client_max_body_size", body_size);
		return 1;
	}

	// Adjust max body size according to the relevant flag
	size_t cmbs = (size_t) stoi(body_size);
	if (flag == KILO)
		cmbs *= 1000;
	else if (flag == MEGA)
		cmbs *= 1000000;
	else if (flag == GIGA)
		cmbs *= 1000000000;

	_client_max_body_size = cmbs;
	return 0;
}

int Location::setIndex(std::list<Node>::iterator &it) {
	for (; it->_type == NodeType::Parameter; it++)
		_index.push_back(it->_content);
	it--;

	// Check there is only 1 argument specified for client_max_body_size
	if (_index.size() < 1) {
		log(std::cerr, MsgType::ERROR, "Invalid number of arguments for", "index");
		_index.clear();
		return 1;
	}

	return 0;
}

int Location::setAutoindex(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == NodeType::Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check valid number of parameters
	if (stash.size() != 1) {
		log(std::cerr, MsgType::ERROR, "Too few arguments for", "autoindex");
		return 1;
	}

	if (stash.back() == "on") {
		_autoindex = true;
		return 0;
	} else if (stash.back() == "off") {
		_autoindex = false;
		return 0;
	}

	log(std::cerr, MsgType::ERROR, "Invalid autoindex parameter", it->_content);
	return 1;
}

int Location::setRoot(std::list<Node>::iterator &it) {
	std::vector<std::string> stash;
	for (; it->_type == NodeType::Parameter; it++)
		stash.push_back(it->_content);
	it--;

	// Check there is only 1 argument specified for client_max_body_size
	if (stash.size() != 1) {
		log(std::cerr, MsgType::ERROR, "Invalid number of arguments for", "root");
		return 1;
	}
	_root = stash.back();
	return 0;
}

