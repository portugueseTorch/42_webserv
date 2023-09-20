import os
import navbar

print('<html>')

print('<head>')
print('<link type="text/css" rel="stylesheet" href="/cgi-bin/style.css" />')
print('<link type="text/css" rel="stylesheet" href="style.css" />')
print('<meta charset="UTF-8">')
print('<title>Good Times</title>')
print('</head>')

print('<body>')

print('<header>')
urls = ['/', '/reviews.py', '/resources']
texts = ['Home', 'Reviews', 'About']
navbar.printNavBar(urls, texts)
print('</header>')

print('<main>')
with os.scandir('./www') as entries:
    for entry in entries:
        if (entry.is_file()):
            print(f'<p>{entry.name}, {entry.path}</p>')
print('</main>')

print('</body>')
print("</html>")
