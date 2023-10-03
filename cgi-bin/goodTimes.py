import os
import navbar

def response():
    response = '<html>'

    response += '<head>'
    response += '<link type="text/css" rel="stylesheet" href="goodTimes.css" />'
    response += '<link type="text/css" rel="stylesheet" href="style.css" />'
    response += '<meta charset="UTF-8">'
    response += '<title>Good Times</title>'
    response += '</head>'

    response += '<body>'

    response += '<header>'
    urls = ['/', '/reviews.py', '/resources']
    texts = ['Home', 'Reviews', 'About']
    response += navbar.text(urls, texts)
    response += '</header>'

    response += '<main>'
    with os.scandir('./www/resources') as entries:
        for entry in entries:
            if (entry.is_file()):
                response += f'<p onclick="deleteFile(this)" style="cursor: pointer">{entry.name}, {entry.path}</p>'

    response += '<form onsubmit="uploadFile(event)">'
    response += '<label for="file-name">File name<input type="text" name="file-name" id="file-name" required></label>'
    response += '<label for="file-content">File content<textarea name="file-content" id="file-content" required></textarea></label>'
    response += '<input id="submit-button" type="submit" value="Add file">'
    response += '</form>'
    response += '</main>'
    response += '<script src="/cgi-bin/goodTimes.js"></script>'
    response += '</body>'
    response += "</html>"

    if os.environ.get("webservMethod") == "POST" :
        print('HTTP/1.1 405 Method Not Allowed')
    else:
        print('HTTP/1.1 200 OK')
    print('Content-type: text/html')
    print(f'Content-Length: {len(response)}')
    print('')
    print(response)
