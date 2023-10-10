#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
# include <string>
# include <fstream>
# include <sstream>
# include <iomanip>
# include <ctime>

# include <list>
# include <vector>
# include <map>
# include <algorithm>

# include <dirent.h>
# include <fcntl.h>
# include <unistd.h>
# include <signal.h>
# include <string.h>
# include <inttypes.h>

# include <arpa/inet.h>
# include <netinet/in.h>

# include <sys/socket.h>
# include <sys/types.h>
# include <sys/select.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <sys/time.h>


# include "HTTPResponse.hpp"
# include "Server.hpp"
# include "ServerEngine/Lexer.hpp"
# include "ServerEngine/Parser.hpp"
# include "ServerEngine/Location.hpp"
# include "ServerEngine/ServerEngine.hpp"
# include "HTTPRequest/HTTPRequest.hpp"

# define RESET					"\x1B[0m"
# define RED					"\x1B[31m"
# define GREEN					"\x1B[32m"
# define YELLOW					"\x1B[33m"
# define CYAN					"\x1B[36m"
# define WHITE					"\x1B[37m"
# define DARK_GREY				"\x1B[90m"
# define LIGHT_RED				"\x1B[91m"
# define LIGHT_GREEN			"\x1B[92m"
# define LIGHT_YELLOW			"\x1B[93m"
# define LIGHT_BLUE				"\x1B[94m"
# define LIGHT_MAGENTA			"\x1B[95m"

# define MAX_EVENTS				10
# define CONNECTION_TIMEOUT		10
# define MAX_LENGTH				4960
# define READ_SET				0
# define WRITE_SET				1
# define ADD_SET				2
# define MOD_SET				4
# define DEL_SET				8

# define DEFAULT_ROOT			"/"
# define DEFAULT_ERROR_FILE		"./www/error_pages/40x.html"

enum MsgType {
	INFO,
	FATAL,
	ERROR,
	WARNING,
	SUCCESS,
	ALL,
};

class ServerEngine;

/****** UTILS.CPP ******/
int						damerauLevenshteinDistance(std::string input, std::string valid);
bool					isindent(int i);
bool					is_file(std::string fname);
bool					is_valid_filename(std::string file_name);
bool					file_is_valid(std::string file_path, int permissions);
void					log(std::ostream &stream, MsgType type, std::string msg, std::string optional);
std::string				readConfigurationFile(int argc, char **argv);
std::string				get_file_extension(std::string file_name);
static std::string* 	nonePointer = NULL;
static std::string*		gContent = NULL;
static ServerEngine* 	gServer = NULL;
void 					handler(int signal);

template <typename T1, typename T2, typename T3>
void cleanUp(T1 *content, T2 *engine, T3 *other){
	if (content)
		delete content;
	if (engine)
		delete engine;
	if (other)
		delete other;
}

#endif
