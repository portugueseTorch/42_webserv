#pragma once
#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <string>
# include <fstream>

# include <list>
# include <vector>
# include <map>
# include <algorithm>

# include <fcntl.h>
# include <unistd.h>
# include <signal.h>
# include <string.h>
# include <inttypes.h>
# include <sys/epoll.h>

# include <arpa/inet.h>
# include <netinet/in.h>

# include <sys/socket.h>
# include <sys/types.h>
# include <sys/select.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <sys/time.h>

# include "Lexer.hpp"
# include "Parser.hpp"
# include "Location.hpp"
# include "Request.hpp"
# include "Server.hpp"
# include "ServerEngine.hpp"

# define RESET			"\x1B[0m"
# define RED			"\x1B[31m"
# define GREEN			"\x1B[32m"
# define YELLOW			"\x1B[33m"
# define CYAN			"\x1B[36m"
# define WHITE			"\x1B[37m"
# define DARK_GREY		"\x1B[90m"
# define LIGHT_RED		"\x1B[91m"
# define LIGHT_GREEN	"\x1B[92m"
# define LIGHT_YELLOW	"\x1B[93m"
# define LIGHT_BLUE		"\x1B[94m"
# define LIGHT_MAGENTA	"\x1B[95m"

# define MAX_EVENTS 10
# define EPOLL_TIMEOUT 1000
# define MAX_LENGTH 4960

enum MsgType {
	INFO,
	FATAL,
	ERROR,
	WARNING,
	SUCCESS,
	ALL,
};

enum Method {
	GET,
	POST,
	HEAD,
	PUT,
	DELETE,
};

/****** UTILS.CPP ******/
int			damerauLevenshteinDistance(std::string input, std::string valid);
bool		isindent(int i);
void		log(std::ostream &stream, MsgType type, std::string msg, std::string optional);
std::string	readConfigurationFile(int argc, char **argv);

#endif
