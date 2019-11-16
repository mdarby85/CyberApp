from api import API
from wsgiref.simple_server import make_server

# Main file that runs the entire app
app = API()

# Route for '/home' path
@app.route("/home")  # Flask-style annotation
def home(request, response):
    response.text = "Hello from the HOME page"

# Route for '/success' path
@app.route("/success")  # Flask-style annotation
def success(request, response):
    response.text = "Hello from the SUCCESS page"

# Test server for running the Python code
httpd = make_server('localhost', 8000, app)
    # print("Serving on port 8000...\nVisit http://127.0.0.1:8000\nTo kill the server we enter 'control + c'")

# Wait for a single request, serve it and quit
httpd.serve_forever()