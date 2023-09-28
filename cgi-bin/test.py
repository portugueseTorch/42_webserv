import os


response = '<html><head><link type="text/css" rel="stylesheet" href="/cgi-bin/style.css" /></head><body>'
response += '<h2>Hello, world</h2>'
response += '<a href="/" class="center button">Go back</a>'
response += "<h2>Printing env variables:</h2>"
for name, value in os.environ.items():
    response += "<p>" + ("{0}: {1}".format(name, value)) + "</p>"
response += '</body></html>'

print('HTTP/1.1 200 OK')
print('Content-type: text/html')
print(f'Content-Length: {len(response)}')
print('')
print(response)