from api import API
import os
import hashlib
from wsgiref.simple_server import make_server

# Main file that runs the entire app
app = API()

# Route for '/' path
@app.route("/")  # Flask-style annotation
def home(request, response):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/index.html').read()
    username = request.params.get('username', 'default')
    passwd = app.encrypt_string(request.params.get('password', 'default'))
    passwd = passwd[:32] # trims hash to 32 chars

    # print(username + " " + passwd)

# Route for '/success' path
@app.route("/success")  # Flask-style annotation
def success(request, response):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/success.html').read()

# Route for '/register' path
@app.route("/register")  # Flask-style annotation
def register(request, response):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/register.html').read()


# Test server for running the Python code
httpd = make_server('localhost', 8000, app)
print("Serving on port 8000...\nVisit http://127.0.0.1:8000\nTo kill the server we enter 'control + c'")

# Wait for a single request, serve it and quit
httpd.serve_forever()