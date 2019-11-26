from api import API
import os
import hashlib
import psycopg2
from webob import Request
from wsgiref.simple_server import make_server

# Main file that runs the entire application
application = API()

# Open db conn
try:
    conn = psycopg2.connect(database="cyberdb", user="postgres", password="postgres", host="localhost", port=5432)
    conn.autocommit = True
except:
    print("Cannot connect to the database!")
    exit(1)

# Route for '/' path
@application.route("/")  # Flask-style annotation
def home(request, response):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/index.html').read()
    
    # The following variables are obtained from the form data through an http POST when 
    # user clicks the submit button
    username = request.params.get('username', 'default')
    passwd = application.encrypt_string(request.params.get('password', 'default'))
    passwd = passwd[:32]  # trims hash to 32 chars

    if username != "default":  # This fixes the problem of redirecting back to a page and it instantly submits the form
        # Check for valid username/password
        query = "SELECT * FROM people WHERE email='{}' AND pwhash='{}'".format(username, passwd)
        try:
            with conn.cursor() as cursor:
                cursor.execute(query)
                if cursor.rowcount > 0:
                    print("Username and Password match!")
                    success(request, response)
                else:
                    print("Invalid Username or Password!")
                    loginerror(response)
                    # print('no redirect')
        except psycopg2.Error as e:
            print(e.pgerror)
    else:
        print("Blank Username or Password on login")

    # # If valid
    # if username == 'darbym':
    #     success(request, response)
    # else:
    #     print('no redirect')

# Route for '/loginerror' path
@application.route("/loginerror")  # Flask-style annotation
def loginerror(response):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/loginerror.html').read()

# Route for '/registererror' path
@application.route("/registererror")  # Flask-style annotation
def registererror(response):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/registererror.html').read()

# Route for '/registersuccess' path
@application.route("/registersuccess")  # Flask-style annotation
def registersuccess(response):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/registersuccess.html').read()

# Route for '/success' path
@application.route("/success")  # Flask-style annotation
def success(request, response):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/success.html').read()

# Route for '/register' path
@application.route("/register")  # Flask-style annotation
def register(request, response):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/register.html').read()
    username = request.params.get('username', 'default')
    passwd = application.encrypt_string(request.params.get('password', 'default'))
    passwd = passwd[:32]

    # check for existing username
    if username != "default":  # This fixes the problem of redirecting back to a page and it instantly submits the form
        query = "SELECT * FROM people WHERE email='{}'".format(username)
        try:
            with conn.cursor() as cursor:
                cursor.execute(query)
                if cursor.rowcount > 0:
                    print("Username already exists!")
                    registererror(response)
                else:
                    query = "INSERT INTO people (email, pwhash) VALUES ('{}', '{}')".format(username, passwd)
                    cursor.execute(query)
                    registersuccess(response)
        except psycopg2.Error:
            print(psycopg2.Error)
    else:
        print("Blank Username or Password on Register")


if __name__ == "__main__":
    # Test server for running the Python code
    httpd = make_server('localhost', 8000, application)
    print("Serving on port 8000...\nVisit http://127.0.0.1:8000\nTo kill the server we enter 'control + c'")
    # Wait for a single request, serve it and quit
    httpd.serve_forever()