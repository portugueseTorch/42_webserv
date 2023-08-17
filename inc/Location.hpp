#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "Webserv.hpp"

class Location
{
	public:
		Location();
		~Location();

		static std::vector<std::string> directives;

		std::string								getLocation() { return _location; }
		std::string								getRoot() { return _root; }
		bool									getAutoindex() { return _autoindex; }
		std::vector<std::string>				&getIndex() { return _index; }
		std::map<int,std::vector<std::string> >	&getErrorPages() { return _error_pages; }

		int setLocation(std::string);
		int setErrorPages(std::list<Node>::iterator &it);
		int setIndex(std::list<Node>::iterator &it);
		int setRoot(std::list<Node>::iterator &it);
		int setAutoindex(std::list<Node>::iterator &it);
		int setHTTPMethod(std::list<Node>::iterator &it);
		int setClientMaxBodySize(std::list<Node>::iterator &it);

	private:
		std::string 							_location;				// location specified by the location directive
		std::map<int,std::vector<std::string> >	_error_pages;			// map storing the error codes and their respective error_pages
		std::vector<std::string>				_index;					// index list according to config_file
		std::string								_root;					// root of the files as provided by the config_file
		bool									_autoindex;				// autoindex as per config_file
		std::vector<int>						_http_method;			// allowed http_methods as per config_file
		size_t									_client_max_body_size;	// lmax size (in bytes) of the client body according to config_file
};

#endif