import os
import sys
import navbar

from cgi import parse_multipart, parse_header
from io import BytesIO
import base64
from urllib.parse import parse_qs
from urllib.parse import unquote

from email import message_from_bytes

def POST() :

    response = '<html>'
    content = os.environ.get("content")
    content = unquote(content)
    content_bytes = content.encode('utf-8')
    contentLength = os.environ.get('content-length')

    sys.stderr.write(f'{content}\n\n')

    test = message_from_bytes(content_bytes)
    response += "test"

    boundary = "--" + os.environ.get("boundary")
    
    sys.stderr.write(f'{boundary}\n')
    # what = test.get_payload().split(boundary + "\r\n")[1]
    # sys.stderr.write(f'{what}\n')
    content = test.get_payload()
    parts = content.split(boundary + "\r\n")
    file_name = ""
    file_content = ""

    for part in parts:
        sys.stderr.write(f'part: {part}')
        if not part or part.isspace():
            continue
        if 'filename=' in part:
            inContent = False
            # Extract the file content
            subparts = part.split('\r')
            for line in subparts:
                if line.isspace() and not inContent:
                    continue
                if 'Content' in line:
                    continue
                else:
                    if not inContent:
                        line = line.strip()
                        inContent = True
                    file_content += line.replace(boundary, '')
                    # Check if the string ends with '--'
                    if file_content.endswith('--'):
                        # Remove the last two characters (--)
                        file_content = file_content[:-2]
        elif 'name="content"; filename' not in part:
            # Extract the file name
            subparts = part.split('\n')
            for line in subparts:
                if line.isspace():
                    continue
                elif 'Content' in line:
                    continue
                else:
                    if not file_name:
                        file_name = line.strip()
                        # sys.stderr.write(f'File Name: {file_name}\n')

    # sys.stderr.write(f'File Content: {file_content}\n')

    file_path = os.path.join('./www/resources', file_name)

    with open(file_path, 'w') as file:
        file.write(file_content)

    # response += f'File Name: {file_name}\n'
    # response += f'File Content: {file_content}'

    # response += str(test)
    response += "</html>"
    # sys.stderr.write(f'{response}\n')

    return response

def GET() :
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
    response += '<label for="file-content">File to upload <input type="file" name="file-content" \
            id="file-content" accept=".txt,.css, .scss,.html,.js" required></label>'
    response += '<input id="submit-button" type="submit" value="Upload">'
    response += '</form>'
    response += '</main>'
    response += '<script src="/cgi-bin/goodTimes.js"></script>'
    response += '</body>'
    response += "</html>"

    return response

def response():

    response = ""

    if os.environ.get("webservMethod") == "POST" :
        response = POST()
        print('HTTP/1.1 200 OK')
    else:
        response = GET()
        print('HTTP/1.1 200 OK')
    
    print('Content-type: text/html')
    print(f'Content-Length: {len(response)}')
    print('')
    print(response)
