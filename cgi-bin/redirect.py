def main(location) :
	headers = 'HTTP/1.1 302 Found\n'
	headers += f'Location: {location}'
	return headers