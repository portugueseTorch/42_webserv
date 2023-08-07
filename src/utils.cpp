#include "../inc/Webserv.hpp"

int isindent(int i) {
	return (i == ' ' || i == '\t');
}

int damerauLevenshteinDistance(std::string input, std::string valid) {
	// Initialize a 2D matrix with all values to 0
	size_t input_len = input.length();
	size_t valid_len = valid.length();
	std::vector<std::vector<int>> d(input_len + 1, std::vector<int>(valid_len + 1, 0));

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

std::string readConfigurationFile(int argc, char **argv) {
	std::ifstream in_file;
	std::string buff;
	std::string content;
	std::string file_name;

	if (argc != 2) {
		file_name = "conf/default.conf";
		log(std::cerr, MsgType::WARNING, "No config_file provided", "");
		log(std::cerr, MsgType::INFO, "Assuming default config_file...", "");
	} else
		file_name = argv[1];

	in_file.open(file_name);
	if (!in_file.is_open()) {
		if (file_name != "conf/default.conf") {
			log(std::cerr, MsgType::ERROR, "Unable to open file", file_name);
			log(std::cerr, MsgType::WARNING, "Assuming default config_file...", "");
		}
		in_file.open("conf/default.conf");
		if (!in_file.is_open()) {
			log(std::cerr, MsgType::FATAL, "Unable to open file", file_name);
			exit (EXIT_FAILURE);
		}
	}

	while (std::getline(in_file, buff)) {
		content += buff;
		if (!in_file.eof())
			content += "\n";
	}
	log(std::cout, MsgType::INFO, "Configuration file successfully read!", "");
	in_file.close();
	return content;
}

void log(std::ostream &stream, MsgType type, std::string msg, std::string optional) {
	switch (type) {
		case MsgType::ERROR:
			stream << LIGHT_RED << "[ERROR]:\t" << msg;
			break;
		
		case MsgType::FATAL:
			stream << RED << "[FATAL]:\t\t" << msg;
			break;

		case MsgType::INFO:
			stream << LIGHT_BLUE << "[INFO]:\t\t" << msg;
			break;
		
		case MsgType::SUCCESS:
			stream << GREEN << "[SUCCESS]:\t" << msg;
			break;

		case MsgType::WARNING:
			stream << LIGHT_YELLOW << "[WARNING]:\t" << msg;
			break;

		default:
			break;
	}

	if (optional != "")
		stream << ": \'" << optional << "\'";
	stream << RESET << std::endl;
}
