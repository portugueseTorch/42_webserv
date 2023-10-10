#include "Webserv.hpp"

/**
 * @brief Checks if I is a whitespace or tab
 * 
 * @param i Character to evaluate
 * @return Returns true if it is whitespace or tab, and false otherwise
 */
bool isindent(int i) {
	return (i == ' ' || i == '\t');
}

/**
 * @brief Calculates the Damerau-Levenshtein Distance between INPUT and
 * VALID
 * 
 * @param input Input to test
 * @param valid Valid directive tp test against
 * @return int representing the distance between the two strings
 */
int damerauLevenshteinDistance(std::string input, std::string valid) {
	// Initialize a 2D matrix with all values to 0
	size_t input_len = input.length();
	size_t valid_len = valid.length();
	std::vector<std::vector<int> > d(input_len + 1, std::vector<int>(valid_len + 1, 0));

	// Set the first row and first collumn with numbers
	for (long unsigned int i = 0; i < (input_len + 1); i++)
		d[i][0] = i;
	for (long unsigned int j = 0; j < (valid_len + 1); j++)
		d[0][j] = j;

	for (long unsigned int i = 1; i < (input_len + 1); i++) {
		for (long unsigned int j = 1; j < (valid_len + 1); j++) {
			int cost = input[i - 1] == valid[j - 1] ? 0 : 1;
			int first_min = std::min(d[i - 1][j] + 1, d[i][j - 1] + 1);
			d[i][j] = std::min(first_min, d[i - 1][j - 1] + cost);

			if (i > 1 && j > 1 && input[i - 1] == valid[j - 2] && input[i - 2] == valid[j - 1])
				d[i][j] = std::min(d[i][j], d[i - 2][j - 2] + cost);
		}
	}
	return d[input_len][valid_len];
}

/**
 * @brief Evaluates ARGC and ARGV to check if a config_file was provided. If so
 * it attempts to open the file, otherwise defaulting to a pre-defined config_file.
 * If opened successfully, reads the contents of the file into a string to be returned
 * 
 * @param argc command-line argument coutner
 * @param argv command-line arguments
 * @return std::string with the contents of the config_file
 */
std::string readConfigurationFile(int argc, char **argv) {
	std::ifstream in_file;
	std::string buff;
	std::string content;
	std::string file_name;

	if (argc != 2) {
		file_name = "conf/default.conf";
		log(std::cerr, WARNING, "No config_file provided", "");
		log(std::cerr, INFO, "Assuming default config_file...", "");
	} else
		file_name = argv[1];

	in_file.open(file_name.c_str());
	if (!in_file.is_open()) {
		if (file_name != "conf/default.conf") {
			log(std::cerr, ERROR, "Unable to open file", file_name);
			log(std::cerr, WARNING, "Assuming default config_file...", "");
		}
		in_file.open("conf/default.conf");
		if (!in_file.is_open()) {
			log(std::cerr, FATAL, "Unable to open file", file_name);
			exit (EXIT_FAILURE);
		}
	}

	while (std::getline(in_file, buff)) {
		content += buff;
		if (!in_file.eof())
			content += "\n";
	}
	log(std::cout, INFO, "Configuration file successfully read!", "");
	in_file.close();
	return content;
}

/**
 * @brief Logs MSG and OPTIONAL to STREAM, according with TYPE.
 * The message is displayed as:
 * [TYPE]:	MSG: 'OPTIONAL'
 * 
 * @param stream output stream where the message will be written
 * @param type type of the message according to MsgType
 * @param msg body of the message to display
 * @param optional optional argument to display
 */
void log(std::ostream &stream, MsgType type, std::string msg, std::string optional) {
	switch (type) {
		case ERROR:
			stream << LIGHT_RED << "[ERROR]:\t" << msg;
			break;
		
		case FATAL:
			stream << RED << "[FATAL]:\t\t" << msg;
			break;

		case INFO:
			stream << LIGHT_BLUE << "[INFO]:\t\t" << msg;
			break;
		
		case SUCCESS:
			stream << GREEN << "[SUCCESS]:\t" << msg;
			break;

		case WARNING:
			stream << LIGHT_YELLOW << "[WARNING]:\t" << msg;
			break;

		default:
			break;
	}

	if (optional != "")
		stream << ": \'" << optional << "\'";
	stream << RESET << std::endl;
}

bool file_is_valid(std::string file_path, int permissions) {
	struct stat s;

	// Check if the file exists
	if (stat(file_path.c_str(), &s) != 0)
		return false;

	// Check if the file has the specified permissions
	return (access(file_path.c_str(), permissions) == 0);
}

bool is_file(std::string fname) {
	return fname != "." && fname != ".." && (fname[0] != '.' && (fname.find('.') != std::string::npos));
}

std::string get_file_extension(std::string file_name) {
	std::string file_extension = "null";
	size_t pos;

	if ((pos = file_name.find_first_of('.')) == file_name.find_last_of('.') && pos != std::string::npos)
		file_extension = file_name.substr(file_name.find('.'));
	return file_extension;
}

bool is_valid_filename(std::string file_name) {
	size_t pos = file_name.find('.');
	return file_name.find_first_of('.') == file_name.find_last_of('.') && pos != std::string::npos;
}
