import os
import csv
from urllib.parse import unquote

def validMethod():
    if (os.environ.get('REQUEST_METHOD') != 'DELETE'):
        print("HTTP/1.1 405 Method Not Allowed")
        print("Content-Type: text/plain")
        print()
        print("Method Not Allowed")
        return False
    return True

def clean_input(input_str):
    return unquote(input_str).replace('+', ' ')

def checkInputs():
	name = os.environ.get("name")
	age = os.environ.get("age")
	location = os.environ.get("location")
	comment = os.environ.get("comment")

	inputs = [ name, age, location, comment ]

	inputs = [clean_input(item) for item in inputs]

	if not name:
		return None
	return inputs

def deleteRow():
	inputs = checkInputs()
	if not inputs:
		return
	
	found = False
	with open('./www/test.csv', "r") as input, open('temp', "w", newline='') as output:
		writer = csv.writer(output)
		for row in csv.reader(input):
			if all(item == row[i] for i, item in enumerate(inputs)) and not found:
				found = True
				continue
			writer.writerow(row)
		output.flush()
	os.replace('temp', './www/test.csv')

if __name__ == "__main__":
	if validMethod():
		deleteRow()