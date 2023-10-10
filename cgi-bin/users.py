import os
import csv
import response as fullResponse
import readFile
import navbar
import errorPage
import sys
from urllib.parse import unquote

def clean_input(input_str):
    return unquote(input_str).replace('+', ' ')

def DELETE() :
    value = os.environ.get("user", "")
    user = value.split('/')
    if not user or not user[0] or not user[1] or not user[2] or not user[3]:
        return 400
    
    user = [clean_input(item) for item in user]
    
    found = False
    database = 'www/test.csv'
    try:
        if (os.path.exists(database)):
            with open(database, "r") as input, open('/tmp/temp', "w", newline='') as output:
                writer = csv.writer(output)
                for row in csv.reader(input):
                    if len(row) != 4:
                        continue
                    if all(item == row[i] for i, item in enumerate(user)) and not found:
                    # if row[3] == email and not found:
                        found = True
                        continue
                    writer.writerow(row)
                output.flush()
            os.replace('/tmp/temp', database)
            return 200
        else:
            return 404
    except PermissionError:
        return 403
    except (FileExistsError, OSError, Exception):
        return 500

def POST() :
    name = os.environ.get("name", "")
    age = os.environ.get("age", "")
    location = os.environ.get("location", "")
    email = os.environ.get("email", "")

    List = [name, age, location, email]

    List = [clean_input(item) for item in List]

    List = [item.rstrip() for item in List]

    sys.stderr.write(f'{name}, {age}, {location}, {email}')

    if name and age and location and email:
        database = 'www/test.csv'
        if (os.path.exists(database)):
            with open(database, 'a', encoding='utf-8') as f_object:
                writer_object = csv.writer(f_object)
                writer_object.writerow(List)
        else:
            return 404

    return 200

def GET() :
    response = '<!DOCTYPE html>\n'
    response += '<html>\n'

    response += '<head>\n'
    response += '<link type="text/css" rel="stylesheet" href="/style.css" />\n'
    response += '<link type="text/css" rel="stylesheet" href="/resources/navbar.css" />\n'
    response += '<style>\n'
    response += readFile.text('cgi-bin/users.css')
    response += '\n</style>'
    response += '<meta charset="UTF-8">\n'
    response += '<title>Users</title>\n'
    response += '</head>\n'

    response += '<body>\n'

    response += '<main>\n'
    response += '<header>\n'
    response += navbar.text()
    response += '</header>\n'
    
    class Person:
        def __init__(self, name, age, location, email):
            self.name = name
            self.age = age
            self.location = location
            self.email = email

    persons = []
    database = 'www/test.csv'
    if (os.path.exists(database)):
        with open(database, 'r') as f:
            reader = csv.reader(f)
            next(reader)
            for row in reader:
                if len(row) != 4:
                    continue
                name, age, location, email = row
                person = Person(name, age, location, email)
                persons.append(person)

        response += '<h2 class="center">We don\'t care about data protection.<br />\n'
        response += 'Email any user you want:</h2>\n'
        response += '<ul>\n'
        for person in persons:
            response += f'<li>\n<div>{person.name}, {person.age}, {person.location}</div>\n \
            <a class="mail-icon" href="mailto: {person.email}">\n \
            <img src="/resources/mail.svg" alt="email icon">\n \
            </a>\n</li>\n'
        response += '</ul>\n'
    else:
        response += '<p style="color: red">Error: cannot access database</p>\n'    
    
    response += '<div class="form-container">\n'
    response += '<div class="add-container center">\n'
    response += '<h2 class="center">Add user:</h2>\n'
    response += '<form class="form" action="users.py" method="post">\n'
    response += '<label for="name">Name: \n'
    response += '<input name="name" type="text" id="user-name" required>\n'
    response += '</label><br />\n'
    response += '<label for="age">Age: \n'
    response += '<input name="age" type="number" min="1" max="150" required>\n'
    response += '</label><br />\n'
    response += '<label for="location">Location: \n'
    response += '<input name="location" type="text" required>\n'
    response += '</label><br />\n'
    response += '<label for="email">Email: \n'
    response += '<input name="email" type="email" required>\n'
    response += '</label><br />\n'
    response += '<input class="submit-button" type="submit" value="Add">\n'
    response += '</form>\n'
    response += '</div>\n'

    response += '<div class="delete-container center">\n'
    response += '<h2 class="center">Delete user:</h2>\n'
    response += '<form class="form" onsubmit="deleteUser(event, this.elements[\'userToDelete\'].value)">\n'
    response += '<select name="userToDelete">\n'
    response += '<option value="" disabled selected>Chose a user</option>\n'
    for person in persons:
        response += f'<option value="{person.name}/{person.age}/{person.location}/{person.email}">{person.name}</option>\n'
    response += '</select>\n'
    response += '<br />\n'
    response += '<input class="submit-button" type="submit" value="Delete">\n'
    response += '</form>\n'
    response += '</div>\n'
    response += '</div>\n'

    response += '</main>\n'
    response += '<script>\n'
    response += readFile.text('cgi-bin/users.js')
    response += '\n</script>'
    response += "</body>\n"
    response += "</html>\n"

    return response

def response():
    response = ""
    headers = ""
    method = os.environ.get("webservMethod")

    if method == "POST" :
        status = POST()
        if status == 200:
            response = GET()
            headers = 'HTTP/1.1 200 OK'
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
            response = GET()
            headers = 'HTTP/1.1 200 OK'
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