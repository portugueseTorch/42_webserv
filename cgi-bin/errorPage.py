import os
import readFile

def GET(status):
	fileName = os.path.join('./www/error_pages', f'{status}.html')
	defaultFileName = './www/error_pages/40x.html'

	if not os.path.exists(fileName):
		fileName = defaultFileName
	
	return readFile.text(fileName)
