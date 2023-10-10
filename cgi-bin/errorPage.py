import os

def GET(status):
	response = ''
	fileName = os.path.join('./www/error_pages', f'{status}.html')
	defaultFileName = './www/error_pages/40x.html'

	try:
		with open(fileName if os.path.exists(fileName) else defaultFileName, 'r') as file:
			response = file.read()
	except Exception as e:
		response = 'Error file cannot be read'

	return response