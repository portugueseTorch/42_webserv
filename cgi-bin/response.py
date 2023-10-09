def display(headers, response):
	headers += f'\nContent-Length: {len(response)}'
	print(headers)
	print('')
	print(response)