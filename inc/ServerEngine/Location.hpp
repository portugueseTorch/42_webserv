#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "Webserv.hpp"

class Location
{
	public:
		Location();
		~Location();

		static std::vector<std::string> directives;
	
		void									displayLocationBlock();

		std::string								getLocation() { return _location; }
		std::string								getRoot() { return _root; }
		size_t									getClientMaxBodySize() { return _client_max_body_size; }
		bool									getAutoindex() { return _autoindex; }
		bool									clientBodySizeSet() { return _body_size_specified; }
		std::vector<std::string>				&getIndex() { return _index; }
		std::vector<std::string>				&getHTTPMethod() { return _http_method; }
		std::map<int,std::vector<std::string> >	&getErrorPages() { return _error_pages; }
		std::pair<int,std::string>				&getReturn() { return _return; }
		bool									getHasReturn() {return _has_return; }

		int setLocation(std::string);
		int setErrorPages(std::list<Node>::iterator &it);
		int setIndex(std::list<Node>::iterator &it);
		int setRoot(std::list<Node>::iterator &it);
		int setAutoindex(std::list<Node>::iterator &it);
		int setHTTPMethod(std::list<Node>::iterator &it);
		int setClientMaxBodySize(std::list<Node>::iterator &it);
		int setReturn(std::list<Node>::iterator &it);

	private:
		static std::string					_possibleDirectives[];
		std::string 							_location;				// location specified by the location directive
		std::map<int,std::vector<std::string> >	_error_pages;			// map storing the error codes and their respective error_pages
		std::pair<int,std::string>				_return;				// stores the code and the related text, if any
		bool									_has_return;			// flag to signal if a 'return' directive was provided
		std::vector<std::string>				_index;					// index list according to config_file
		std::string								_root;					// root of the files as provided by the config_file
		bool									_autoindex;				// autoindex as per config_file
		std::vector<std::string>				_http_method;			// allowed http_Method as per config_file
		size_t									_client_max_body_size;	// lmax size (in bytes) of the client body according to config_file
		bool									_body_size_specified;	// flag to signal if a max body size was specified in the confug_file
};

#endif
