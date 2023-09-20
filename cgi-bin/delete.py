import os
import csv

name = os.environ.get("name")
found = False
if name:
    with open('./www/test.csv', "r") as input, open('temp', "w", newline='') as output:
        writer = csv.writer(output)
        for row in csv.reader(input):
            if name == row[0] and not found:
                found = True
                continue
            writer.writerow(row)
        output.flush()
    os.replace('temp', './www/test.csv')
