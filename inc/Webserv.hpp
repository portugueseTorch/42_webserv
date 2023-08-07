#pragma once
#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <list>
# include <string>
# include <vector>
# include <string>
# include <fstream>
# include <unistd.h>
# include <iostream>
# include <algorithm>
# include <vector>
# include <algorithm>

# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <inttypes.h>

# include <sys/types.h>
# include <sys/select.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <signal.h>


# include "Lexer.hpp"
# include "Parser.hpp"
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

enum MsgType {
	INFO,
	FATAL,
	ERROR,
	WARNING,
	SUCCESS,
};

/****** UTILS.CPP ******/
int			isindent(int i);
int			damerauLevenshteinDistance(std::string input, std::string valid);
std::string	readConfigurationFile(int argc, char **argv);
void		log(std::ostream &stream, MsgType type, std::string msg, std::string optional);

#endif