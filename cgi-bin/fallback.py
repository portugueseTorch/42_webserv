import os
import response as fullResponse
import readFile
import navbar

def response() :

    response = '<!DOCTYPE html>\n'
    
    response += '<html>\n'
    response += '<head>\n'
    response += '<link type="text/css" rel="stylesheet" href="/style.css" />\n'
    response += '<link type="text/css" rel="stylesheet" href="/resources/navbar.css" />\n'
    response += '<style>\n'
    response += readFile.text('cgi-bin/fallback.css')
    response += '\n</style>'
    response += '<meta charset="UTF-8">\n'
    response += '<title>Good Times</title>\n'
    response += '</head>\n'

    response += '<body>\n'
    response += '<header>\n'
    
    response += navbar.text()
    response += '</header>\n'
    response += '<h2 class="center">This is the CGI index page</h2>\n'
    response += '<p class="center">Nothing else to show here...</p>\n'
    response += '</body>\n'
    response += "</html>\n"
    
    headers = 'HTTP/1.1 200 OK'
    fullResponse.display(headers, response)


if __name__ == "__main__" :
    response()