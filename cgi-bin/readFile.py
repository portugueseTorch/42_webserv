import os

def text(fileName):
    try:
        with open(fileName, 'r') as file:
            response = file.read()
    except Exception as e:
        response = f'File {fileName} cannot be read from {os.getcwd()}'
    
    return response