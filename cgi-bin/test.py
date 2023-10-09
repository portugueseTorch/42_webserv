import os
import response as fullResponse

def response() :

    response = '<html><head><link type="text/css" rel="stylesheet" href="/cgi-bin/style.css" /></head><body>'
    response += '<h2>Hello, world</h2>'
    response += '<a href="/" class="center button">Go back</a>'
    response += "<h2>Printing env variables:</h2>"
    for name, value in os.environ.items():
        response += "<p>" + ("{0}: {1}".format(name, value)) + "</p>"
    response += '</body></html>'
    
    headers = 'HTTP/1.1 200 OK'
    fullResponse.display(headers, response)


if __name__ == "__main__" :
    response()