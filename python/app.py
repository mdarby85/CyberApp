import os
from wsgiref.simple_server import make_server

def web_app(environment, response):

    # Get File Contents
    file_contents = b""
    with open("./templates/index.html", "rb") as file:
        file_contents = file.read()

    # Add Dynamic Content
    response_body = b"This is a header!".join(
        b"".join(
            file_contents.split(b"%HEAD)")
        ).split(b"%(HEADING)")
    )

    # Heading
    status = '200 OK'
    headers = [
        ('Content-type', 'text/html; charset=utf-8'), ('Content-Length', str(len(response_body)))
        ]

    # Send Response
    response(status, headers)
    return [response_body]

with make_server('', 8000, web_app) as server:
    print("Serving on port 8000...\nVisit http://127.0.0.1:8000\nTo kill the server we enter 'control + c'")

    server.serve_forever()