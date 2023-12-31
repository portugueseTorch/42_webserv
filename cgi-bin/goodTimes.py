import os
import readFile
import html
import navbar
import response as fullResponse
import errorPage
import redirect

from urllib.parse import unquote
from email import message_from_bytes

def DELETE() :
    fileName = os.environ.get('webservFileName')
    
    status = 200
    if fileName is None:
        status = 400
    else:
        filePath = os.path.join('./www/resources', fileName)

        try:
            if os.path.exists(filePath):
                st = os.stat(filePath)
                perm = st.st_mode
                if (perm == 33060):
                    raise PermissionError()
                os.remove(filePath)
            else:
                status = 404
        except FileNotFoundError:
            status = 404
        except PermissionError:
            status = 403
        except OSError:
            status = 500
    return status

def POST() :
    content = os.environ.get("content")
    if content is None:
        return 400
    content = unquote(content)
    content_bytes = content.encode('utf-8')

    test = message_from_bytes(content_bytes)

    boundary = "--" + os.environ.get("boundary")
    if boundary is None:
        return 400
    
    content = test.get_payload()
    parts = content.split(boundary + "\r\n")
    file_name = ""
    file_content = ""

    for part in parts:
        if not part or part.isspace():
            continue
        if 'name="content"' in part:
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
                    toAdd = line.replace(boundary, '')
                    file_content += toAdd
                    # Check if the string ends with '--'
                    if file_content.endswith('--'):
                        # Remove the last two characters (--)
                        file_content = file_content[:-2]
        elif 'name="content"' not in part:
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

    if not file_name:
        return 400
    
    file_path = os.path.join('./www/resources', file_name)

    try:
        with open(file_path, 'w') as file:
            file.write(file_content)
    except FileNotFoundError:
        return 404
    except PermissionError:
        return 403
    except IsADirectoryError:
        return 500
    except OSError:
        return 500

    return 200

def GET(fileContent = "No file selected") :
    response = '<!DOCTYPE html>\n'
    
    response += '<html>\n'
    response += '<head>\n'
    response += '<link type="text/css" rel="stylesheet" href="/style.css" />\n'
    response += '<link type="text/css" rel="stylesheet" href="/resources/navbar.css" />\n'
    response += '<style>\n'
    response += readFile.text('cgi-bin/goodTimes.css')
    response += '\n</style>'
    response += '<meta charset="UTF-8">\n'
    response += '<title>Resources</title>\n'
    response += '</head>\n'

    response += '<body>\n'
    response += '<header>\n'
    response += navbar.text()
    response += '</header>\n'
    response += '<main>\n'
    response += '<form class="upload-form" onsubmit="uploadFile(event)">\n'
    response += '<label for="file-content">Choose a simple file, text only</label> \
                <input type="file" name="file-content" id="file-content" accept=".txt,.css,.scss,.html,.js,.svg" required>\n'
    response += '<input class="submit-button" type="submit" value="Upload">\n'
    response += '</form>\n'
    response += '<div class="file-list">\n'
    response += '<h2>Uploaded Files</h2>\n'
    response += '<ul id="uploaded-files">\n'
    with os.scandir('./www/resources') as entries:
        for entry in entries:
            if entry.is_file():
                response += f'<li class="uploaded-file" onclick="displayFile(this.querySelector(\'.file-info\'), this)">\n \
                    <span class="file-info">{entry.name}</span>\n \
                          <img class="trash-icon" src="/resources/trash.svg" alt="Delete" \
                            onclick="deleteFile(this.previousElementSibling.innerText, this.parentElement); event.stopPropagation();">\n \
                            </li>'
    response += '</ul>\n'
    response += '</div>\n'

    response += '<div class="file-contents" id="file-content">\n'
    response += '<h2 onclick="clearOutput()">File Content</h2>\n'
    fileContent = html.escape(fileContent)
    fileContent = fileContent.replace('\n', '<br />')
    response += f'<div class="content" id="file-content-text">{fileContent}</div>\n'
    response += '</div>\n'

    response += '</main>\n'
    response += '<script>\n'
    response += readFile.text('cgi-bin/goodTimes.js')
    response += '\n</script>\n'
    response += '</body>\n'
    response += "</html>\n"

    return response

def response():

    response = ""
    headers = ""
    method = os.environ.get("webservMethod")

    if method == "POST" :
        status = POST()
        if status == 200:
            headers = redirect.main('/goodTimes.py')
        else:
            if status == 403:
                headers = 'HTTP/1.1 403 Forbidden'
            elif status == 404:
                headers = 'HTTP/1.1 404 Not Found'
            elif status == 500:
                headers = 'HTTP/1.1 500 Internal Server Error'
            else:
                headers = 'HTTP/1.1 400 Bad Request'
            response = errorPage.GET(status)
    elif method == "DELETE":
        status = DELETE()
        if status == 200:
            headers = redirect.main('/goodTimes.py')
        else:
            if status == 403:
                headers = 'HTTP/1.1 403 Forbidden'
            elif status == 404:
                headers = 'HTTP/1.1 404 Not Found'
            elif status == 500:
                headers = 'HTTP/1.1 500 Internal Server Error'
            else:
                headers = 'HTTP/1.1 400 Bad Request'
            response = errorPage.GET(status)
    else:
        response = GET()
        headers = 'HTTP/1.1 200 OK'
    
    fullResponse.display(headers, response)
