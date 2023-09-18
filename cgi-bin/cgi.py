import os
import csv

name = os.environ.get("name")

print('<html><head><link type="text/css" rel="stylesheet" href="/cgi-bin/style.css" /></head><body>')
if name:
	name = name.replace('%20', ' ')
	print('<h1 class="success center">We have a name!</h1>')
	print(f'<p class="center">Welcome, {str.title(name)}</p>')
else:
	print('<h2 class="center">No name found...</h2>')
	print('<h1 class="failure center">For the love of god, just work!</h1>')

user = os.environ.get("user")
pwd = os.environ.get("pass")
print(f'<p>User: {user}<br/>Password: {pwd}</p>')

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

print('<h2>Details:</h2>')
print('<ul>')
for person in persons:
	print(f'<li>{person.name}, {person.age}</li>')
print('</ul>')
print('<a href="/" class="center">Go back</a>')
print("</body></html>")