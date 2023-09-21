from csv import writer
import os
from urllib.parse import unquote

def clean_input(input_str):
    return unquote(input_str).replace('+', ' ')

name = os.environ.get("name", "")
age = os.environ.get("age", "")
location = os.environ.get("location", "")
comment = os.environ.get("comment", "")

List = [name, age, location, comment]

List = [clean_input(item) for item in List]

# Remove trailing whitespaces
List = [item.rstrip() for item in List]

if name:
	with open('./www/test.csv', 'a', encoding='utf-8') as f_object:
		writer_object = writer(f_object)
		writer_object.writerow(List)

print("HTTP/1.1 303 See Other")
print("Location: http://localhost:8080/cgi-bin")
print("Content-Length: 0")
print()
