def text():
    
    urls = ['/', '/goodTimes.py', '/users.py', '/about']
    texts = ['Home', 'Resources', 'Users', 'About']

    response = '<div class="wrapper">'
    response += '<h1><a class="main-title" href="/">Webserving Delight</a></h1>'
    response += '<p>Explore the Wonders of a web server</p>'
    response += '<nav class="navbar">'
    response += f'<a href="{urls[0]}">{texts[0]}</a>'
    response += '<div class="separator">|</div>'
    response += f'<a href="{urls[1]}">{texts[1]}</a>'
    response += '<div class="separator">|</div>'
    response += f'<a href="{urls[2]}">{texts[2]}</a>'
    response += '<div class="separator">|</div>'
    response += f'<a href="{urls[3]}">{texts[3]}</a>'
    response += '</nav>'
    response += '</div>'

    return response