from api import API
import os
import hashlib
import psycopg2
from webob import Request
from wsgiref.simple_server import make_server

# Main file that runs the entire application
application = API()

# Open db conn
#conn = psycopg2.connect(database="cyberdb", user="postgres", password="postgres", host="localhost")
#cur = conn.cursor()

# Route for '/' path
@application.route("/")  # Flask-style annotation
def home(request, response):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/index.html').read()
    
    # The following variables are obtained from the form data through an http POST when 
    # user clicks the submit button
    username = request.params.get('username', 'default')
    passwd = application.encrypt_string(request.params.get('password', 'default'))
    passwd = passwd[:32] # trims hash to 32 chars

    ## query goes here ##

    #####################

    ## compare results of query for valid login ##

    ##############################################

    # If valid
    if username == 'darbym':
        success(request, response)
    else:
        print('no redirect')

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
    # cur.execute("INSERT INTO people (username)")

    # con.commit()
    # con.close()




# Test server for running the Python code
httpd = make_server('localhost', 8000, application)
print("Serving on port 8000...\nVisit http://127.0.0.1:8000\nTo kill the server we enter 'control + c'")

# Wait for a single request, serve it and quit
httpd.serve_forever()