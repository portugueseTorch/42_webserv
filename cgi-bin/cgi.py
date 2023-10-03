import os
import fallback
import test
import goodTimes

if __name__ == "__main__" :
	path = os.environ.get('webservPath')

	if path == "test.py":
		test.response()
	elif path == "goodTimes.py":
		goodTimes.response()
	else:
		fallback.response()