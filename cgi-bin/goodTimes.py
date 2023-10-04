import os
import navbar

from cgi import parse_multipart, parse_header
from io import BytesIO
import base64
from urllib.parse import parse_qs



def POST() :
    response = '<html>'

    content = os.environ.get("content")
    boundary = os.environ.get("boundary")
    contentLength = os.environ.get("content-length")

    # c_type, c_data = parse_header("multipart/form-data; " + boundary)
    # assert c_type == 'multipart/form-data'
    # c_data = {'boundary': boundary}
    # decoded_string = base64.b64decode(content)
    # c_data['boundary'] = bytes(c_data['boundary'], "utf-8")
    # c_data['CONTENT-LENGTH'] = int(contentLength)
    # form_data = parse_multipart(BytesIO(decoded_string), c_data)

    # length = int(contentLength)
    # body = content.read(length).decode()
    # params = parse_qs(content)

    # messagecontent = params["message"][0]
    # response += messagecontent

    import re, io
    with io.StringIO(content) as fh:
        parts = []
        part_line = "[]"
        part_fname = ""
        new_part = None
        # robj = re.compile('.+name=+')

        while True:
            line = fh.readline()
            if not line: break
            if not new_part:
                new_part = line[:-1]

            if line.startswith(new_part):
                if part_line:
                    parts.append({'filename':part_fname, 'content':''.join(part_line)})
                    part_line = []

                while line and line != '\n':
                    # response += line
                    # _match = robj.match(line)
                    # response += "\n" + str(_match) + "\n"
                    part_line += line
                    # if _match: part_fname = _match.groups()[0]
                    line = fh.readline()
            # else:
            #     part_line.append(line)

    for part in parts:
        response += str(part)

    # response += c_data['CONTENT-LENGTH']
    # response += "<br />"
    # response += str(len(form_data))
    # response += "<br />"
    # c_data_list = list(c_data.items())
    # for image_str in c_data_list:
    #     response += str(image_str)
    # response += content
    response += "</html>"

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
    response += '<label for="file-content">File to upload <input type="file" name="file-content" id="file-content" required></label>'
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
