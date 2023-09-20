import os
import csv


name = os.environ.get("name")
if name:
    with open('./www/test.csv', "r") as input, open('temp', "w") as output:
        writer = csv.writer(output)
        for row in csv.reader(input):
            # print(row)
            if name != row[0]:
                writer.writerow(row)
        input.close()
        output.close()

    with open('temp', "r") as input, open('./www/test.csv', "w") as output:
        writer = csv.writer(output)
        for row in csv.reader(input):
            writer.writerow(row)
        input.close()
        output.close()
        os.remove('temp')

print("HTTP/1.1 303 See Other")
print("Location: http://localhost:8080/cgi-bin")
print("Content-Length: 0")
print()