from api import API
import os
import hashlib
import psycopg2
from webob import Request
# from wsgiref.simple_server import make_server

NOT_FRIEND = 0
PEND_FRIEND = 1
IS_FRIEND = 2

# Main file that runs the entire application
application = API()

# Open db conn
connectionInfo = {
    "database": "",
    "user": "",
    "password": "",
    "host": "",
    "port": ""
}


def getDBConnectionInfo():
    with open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'DBInfo')) + '/secret.txt', "r") as file:
        connectionInfo["database"] = file.readline().rsplit(':')[1].split("\n")[0]
        connectionInfo["user"] = file.readline().rsplit(':')[1].split("\n")[0]
        connectionInfo["password"] = file.readline().rsplit(':')[1].split("\n")[0]
        connectionInfo["host"] = file.readline().rsplit(':')[1].split("\n")[0]
        connectionInfo["port"] = file.readline().rsplit(':')[1].split("\n")[0]


try:
    getDBConnectionInfo()
    conn = psycopg2.connect(
                            database=connectionInfo["database"],
                            user=connectionInfo["user"],
                            password=connectionInfo["password"],
                            host=connectionInfo["host"],
                            port=connectionInfo["port"]
                           )
    conn.autocommit = True
except psycopg2.Error as e:
    print("Cannot connect to the database!")
    print(connectionInfo)
    print(e)
    print(e.pgcode)
    print(e.pgerror)
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
                    success(request, response, username)
                else:
                    print("Invalid Username or Password!")
                    loginerror(response)
                    # print('no redirect')
        except psycopg2.Error as e:
            print(e.pgerror)
    else:
        print("Blank Username or Password on login")

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
def success(request, response, useremail):
    pendingfriends = htmlFriendsTable(useremail, PEND_FRIEND)
    friends = htmlFriendsTable(useremail, IS_FRIEND)
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/success.html').read().format(user=useremail,
                                                          pending=pendingfriends,
                                                          friends=friends,
                                                          location=getPosition(useremail),
                                                          availablefriends=htmlListAvailableFriends(useremail))

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


@application.route("/valueupdated")  # Flask-style annotation
def valueupdated(response):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/valueupdated.html').read()


@application.route("/updateerror")  # Flask-style annotation
def updateerror(response, errmsg):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/updateerror.html').read().format(message=errmsg)

# Route for '/updateusername' path
@application.route("/updateusername")  # Flask-style annotation
def updateusername(request, response):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/updateusername.html').read()

    currentusername = request.params.get('currentusername')
    currentpass = request.params.get('currentpassword')
    if currentpass is not '' and currentpass is not None:
        currentpass = application.encrypt_string(currentpass)[:32]
    newusername = request.params.get('newusername')

    if (currentusername is not '') and (currentpass is not '') and (newusername is not '') and (currentusername is not None) and (currentpass is not None) and (newusername is not None):
        validinfo = False
        usernameavailable = False
        # Check if login is valid
        if currentusername != "default":
            # Check for valid username/password
            query = "SELECT * FROM people WHERE email='{}' AND pwhash='{}'".format(currentusername, currentpass)
            try:
                with conn.cursor() as cursor:
                    cursor.execute(query)
                    if cursor.rowcount > 0:
                        print("Username and Password match!")
                        validinfo = True
                    else:
                        updateerror(response, "Invalid Username or Password! Cannot change your username.")
            except psycopg2.Error as e:
                print(e.pgerror)
        else:
            print("Blank Username or Password on Valid Credential Check")

        # Check if new username is available
        query = "SELECT * FROM people WHERE email='{}'".format(newusername)
        try:
            with conn.cursor() as cursor:
                cursor.execute(query)
                if cursor.rowcount > 0:
                    updateerror(response, "Your new username already exists! Please choose another.")
                else:
                    usernameavailable = True
        except psycopg2.Error:
            print(psycopg2.Error)

        if validinfo and usernameavailable:
            # Update Login Table
            query1 = "UPDATE people SET email = '{}' WHERE email = '{}'".format(newusername, currentusername)

            # Update Friend Table
            query2 = "UPDATE knownpeople SET email = '{}' WHERE email = '{}'".format(newusername, currentusername)
            query3 = "UPDATE knownpeople SET friendemail = '{}' WHERE friendemail = '{}'".format(newusername, currentusername)

            # Update Location Table
            query4 = "UPDATE position SET email = '{}' WHERE email = '{}'".format(newusername, currentusername)

            try:
                with conn.cursor() as cursor:
                    cursor.execute(query1)
                    cursor.execute(query2)
                    cursor.execute(query3)
                    cursor.execute(query4)
            except psycopg2.Error:
                print(psycopg2.Error)
            print("Successfully Updated Username {} to {}".format(currentusername, newusername))
            valueupdated(response)
    else:
        print("Empty Values in Change Username")


# Route for '/updatepassword' path
@application.route("/updatepassword")  # Flask-style annotation
def updatepassword(request, response):
    response.text = open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
                         + '/updatepassword.html').read()
    passmatch = False
    validinfo = False

    # Pull values from form
    currentusername = request.params.get('currentusername', 'default')
    currentpass = request.params.get('currentpassword', 'default')

    newpass = request.params.get('newpassword')
    confirmpass = request.params.get('confirmpassword')

    if currentusername != 'default' and currentpass != 'default':

        # Check if new password matches confirmation
        if newpass == confirmpass:
            if newpass is not None and len(newpass) > 0:
                passmatch = True
            else:
                updateerror(response, "Your new password cannot be blank!")
        else:
            updateerror(response, "Your new password did not match the confirmation!")
        # Check if User info is correct
        if currentpass is not '' and currentpass is not None:
            currentpass = application.encrypt_string(currentpass)[:32]

        if (currentusername is not '') and (currentpass is not '') and (currentusername is not None) and (currentpass is not None):
            # Check if login is valid
            if currentusername != "default":
                # Check for valid username/password
                query = "SELECT * FROM people WHERE email='{}' AND pwhash='{}'".format(currentusername, currentpass)
                try:
                    with conn.cursor() as cursor:
                        cursor.execute(query)
                        if cursor.rowcount > 0:
                            print("Username and Password match!")
                            validinfo = True
                        else:
                            updateerror(response, "Invalid Username or Password! Cannot change your password.")
                except psycopg2.Error as e:
                    print(e.pgerror)
            else:
                print("Blank Username or Password on Valid Credential Check")

            if validinfo and passmatch:
                if newpass is not '' and newpass is not None:
                    newpass = application.encrypt_string(newpass)[:32]

                # Update Login Table
                query = "UPDATE people SET pwhash = '{}' WHERE email = '{}'".format(newpass, currentusername)
                try:
                    with conn.cursor() as cursor:
                        cursor.execute(query)
                except psycopg2.Error:
                    print(psycopg2.Error)
                print("Successfully Updated Password!")
                valueupdated(response)
        else:
            print("Empty Values in Change Password")
    else:
        print("Empty Submit For Change Password")



def htmlFriendsTable(useremail, friendstatus):
    query = "SELECT email FROM knownpeople WHERE friendemail='{}' AND status={}".format(useremail, friendstatus)
    fullStr = "<table border=\"1\"><tr>"
    try:
        with conn.cursor() as cursor:
            cursor.execute(query)
            colnames = [desc[0] for desc in cursor.description]
            fullStr += ''.join("<th>{}</th>".format(x) for x in colnames)
            fullStr += "</tr>"
            for record in cursor:
                fullStr += "<tr>"
                fullStr += ''.join("<td>{}</td>".format(x) for x in record)
                fullStr += "</tr>"
    except psycopg2.Error as e:
        print(e.pgerror)
    fullStr += "<table>"
    return fullStr


def getPosition(useremail):
    query = "SELECT lat, long FROM position WHERE email='{}'".format(useremail)
    try:
        with conn.cursor() as cursor:
            cursor.execute(query)
            location = cursor.fetchone()
            if location is None:
                return "No Location Exists for user: {}".format(useremail)
    except psycopg2.Error as e:
        print(e.pgerror)
    return "[Latitude: {}] [Longitude: {}]".format(location[0], location[1])


def htmlListAvailableFriends(useremail):
    # Get list of all users in Database
    query = "SELECT email FROM people"
    availableFriends = []
    temp_list = []
    try:
        with conn.cursor() as cursor:
            cursor.execute(query)
            colnames = [desc[0] for desc in cursor.description]
            for record in cursor:
                availableFriends.append(record[0])

            # Get list of pending friends and friends
            query = "SELECT email FROM knownpeople WHERE friendemail='{}'".format(useremail)
            cursor.execute(query)
            # Remove friends from all users list
            for record in cursor:
                temp_list.append(record[0])
                availableFriends.remove(record[0])
    except psycopg2.Error as e:
        print(e.pgerror)

    # Add html tags to list
    fullStr = "<table border=\"1\"><tr>"
    fullStr += ''.join("<th>{}</th>".format(x) for x in colnames)
    fullStr += "</tr>"
    for value in availableFriends:
        fullStr += "<tr>"
        fullStr += ''.join("<td>{}</td>".format(value))
        fullStr += "</tr>"
    return fullStr


# if __name__ == "__main__":
#     # Test server for running the Python code
#     httpd = make_server('localhost', 8000, application)
#     print("Serving on port 8000...\nVisit http://127.0.0.1:8000\nTo kill the server we enter 'control + c'")
#     # Wait for a single request, serve it and quit
#     httpd.serve_forever()
