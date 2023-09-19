import os
import csv

print('<html>')

print('<head>')
print('<link type="text/css" rel="stylesheet" href="/cgi-bin/style.css" />')
print('<link type="text/css" rel="stylesheet" href="style.css" />')
print('<meta charset="UTF-8">')
print('<title>Reviews</title>')
print('</head>')

print('<body>')

print('<header>')
print('<nav class="navbar">')
print('<a href="/">Home</a>')
print('<div class="separator">|</div>')
print('<a href="/resources/index.html">Good times</a>')
print('<div class="separator">|</div>')
print('<a href="/resources">About</a>')
print('</nav>')
print('</header>')

class Person:
	def __init__(self, name, age, location, comment):
		self.name = name
		self.age = age
		self.location = location
		self.comment = comment

#this needs to be more dynamic, database file needs to be param somehow
persons = []
with open('./www/test.csv', 'r') as f:
	reader = csv.reader(f)
	next(reader)
	for row in reader:
		name, age, location, comment = row
		person = Person(name, age, location, comment)
		persons.append(person)

print('<h1 class="page-title">Reviews</h1>')
print('<div class="reviews">')

print('<div class="person reviews-header">')
print('<span>Name</span>')
print('<span>Age</span>')
print('<span>Location</span>')
print('<span>Comment</span>')
print('</div>')
for person in persons:
	print('<div class="person">')
	print(f'<span>{person.name}</span>')
	print(f'<span>{person.age}</span>')
	print(f'<span>{person.location}</span>')
	print(f'<span>{person.comment}</span>')
	print('</div>')
print('</div>')

print('<form action="append.py" method="post">')
print('<label for="name">Name: <input type="text" name="name" required></label>')
print('<label for="age">Age: <input type="number" name="age"></label>')
print('<label for="location">Location: <input type="text" name="location"></label>')
print('<label for="comment">Leave a comment: <textarea name="comment" required> </textarea></label>')
print('<input id="submit-button" type="submit" value="Add review">')
print('</form>')

print("</body>")
print("</html>")
