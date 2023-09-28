import os

response = "<html>"

response += '<head>'
response += '<link type="text/css" rel="stylesheet" href="/cgi-bin/style.css" />'
response += '<link type="text/css" rel="stylesheet" href="style.css" />'
response += '<meta charset="UTF-8">'
response += '<title>Good Times</title>'
response += '</head>'

response += '<body>'

response += '<main>'
with os.scandir('./www/resources/about') as entries:
    for entry in entries:
        if (entry.is_file()):
            response += f'<p style="cursor: pointer">{entry.name}, {entry.path}</p>'

response += '<form onsubmit="uploadFile(event)">'
# response += '<label for="file-name">File name<input type="text" name="file-name" id="file-name" required></label>'
response += '<label for="file-content">File content<input type="file" name="file-content" id="file-content" required></textarea></label>'
response += '<input id="submit-button" type="submit" value="Add file">'
response += '</form>'
response += '</main>'
response += '<script src="/cgi-bin/goodTimes.js"></script>'
response += '</body>'
response += "</html>"

print('HTTP/1.1 200 OK')
print('Content-type: text/html')
print(f'Content-Length: {len(response)}')
print('')
print(response)