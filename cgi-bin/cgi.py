import os

name = os.environ.get("name")
print('<html><head><link type="text/css" rel="stylesheet" href="/cgi-bin/style.css" /></head><body>')
if name:
	print('<h1 class="success">We have a name!</h1>')
	print(f"<p>Welcome, {str.title(name)}</p>")
else:
	print("<h2>No name found...</h2>")
	print('<h1 class="failure">For the love of god, just work!</h1>')

user = os.environ.get("user")
pwd = os.environ.get("pass")
print(f'<p>User: {user}<br/>Password: {pwd}</p>')
print('<a href="/">Go back</a>')
print("</body></html>")