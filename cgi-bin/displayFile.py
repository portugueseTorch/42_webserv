import os
import goodTimes
import response as fullResponse
import errorPage
import readFile

def response():
    status = 200
    fileName = os.environ.get('webservFileName')
    if fileName is None:
        status = 500
    else:
        filePath = os.path.join('./www/resources', fileName)
        try:
            if os.path.exists(filePath):
                with open(filePath, 'r') as file:
                    content = readFile.text(filePath)
            else:
                status = 500
        except Exception as e:
            status = 500

    if status == 500:
        headers = 'HTTP/1.1 500'
        response = errorPage.GET(status)
    else:
        headers = 'HTTP/1.1 200 OK'
        response = goodTimes.GET(content)
    fullResponse.display(headers, response)

