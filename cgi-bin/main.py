import os
import fallback
import users
import goodTimes
import displayFile

if __name__ == "__main__" :
	path = os.environ.get('webservPath')

	path_to_function = {
		"users.py": users.response,
		"goodTimes.py": goodTimes.response,
		"displayFile.py": displayFile.response,
	}

	response_function = path_to_function.get(path, fallback.response)

	response_function()