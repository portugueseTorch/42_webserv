import os
import csv

response = '<html><head><link type="text/css" rel="stylesheet" href="/cgi-bin/style.css" /></head><body>'
response += '<header>'
response += '<nav class="navbar">'
response += '<a href="/">Home</a>'
response += '<div class="separator">|</div>'
response += '<a href="/resources">Potatoes</a>'
response += '</nav>'
response += '</header>'

name = os.environ.get("name")
if name:
	name = name.replace('%20', ' ')
	response += '<h1 class="success center">We have a name!</h1>'
	response += '<p class="center">Welcome, ' + str.title(name) + '</p>'
else:
	response += '<h2 class="center">No name found...</h2>'
	response += '<h1 class="failure center">For the love of god, just work!</h1>'

user = os.environ.get("user")
pwd = os.environ.get("pass")
if user and pwd:
	response += '<p>User: ' + user + '<br/>Password: ' + pwd + '</p>'

class Person:
    def __init__(self, name, age, location, email):
        self.name = name
        self.age = age
        self.location = location
        self.email = email

#this needs to be more dynamic, database file needs to be param somehow
persons = []
with open('./www/test.csv', 'r') as f:
	reader = csv.reader(f)
	next(reader)
	for row in reader:
		name, age, location, email = row
		person = Person(name, age, location, email)
		persons.append(person)

response += '<h2>Details:</h2>'
response += '<ul>'
for person in persons:
	response += '<li>' + person.name + ', ' + person.age + '</li>'
response += '</ul>'
response += '<a href="/" class="center button">Go back</a>'
response += "</body></html>"

print('HTTP/1.1 200 OK')
print('Content-type: text/html')
print(f'Content-Length: {len(response)}')
print('')
print(response)