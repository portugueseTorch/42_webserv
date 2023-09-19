from csv import writer
import os

name = os.environ.get("name") if os.environ.get("name") else ""
age = os.environ.get("age") if os.environ.get("age") else ""
location = os.environ.get("location") if os.environ.get("location") else ""
comment = os.environ.get("comment") if os.environ.get("comment") else ""

name = name.replace('+', ' ')
location = location.replace('+', ' ')
comment = comment.replace('+', ' ')
List = [name, age, location, comment]

if name:
	with open('./www/test.csv', 'a', encoding='utf-8') as f_object:
		writer_object = writer(f_object)
		writer_object.writerow(List)
		f_object.close()

print("HTTP/1.1 303 See Other")
print("Location: http://localhost:8080/cgi-bin")
print("Content-Length: 0")
print()
