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
with os.scandir('./www/resources') as entries:
    for entry in entries:
        if (entry.is_file()):
            print(f'<p onclick="deleteFile(this)" style="cursor: pointer">{entry.name}, {entry.path}</p>')

print('<form onsubmit="uploadFile(event)">')
print('<label for="file-name">File name<input type="text" name="file-name" id="file-name" required></label>')
print('<label for="file-content">File content<textarea name="file-content" id="file-content" required></textarea></label>')
print('<input id="submit-button" type="submit" value="Add file">')
print('</form>')
print('</main>')
print('<script src="/cgi-bin/goodTimes.js"></script>')
print('</body>')
print("</html>")
