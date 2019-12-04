#!/usr/bin/env python

"""
Main web app server functionality.
"""

import cgi
import os
from lib import *       # Our custom framework library
from routes import *    # Website routes

# DEBUG MODE - Remove from live version
import cgitb  # CGI Traceback Manager in Browser
cgitb.enable()


# Get environment variables
REQUEST_METHOD = os.environ['REQUEST_METHOD'].lower()
URL_PATH = "/"
#URL_PATH = os.environ['REQUEST_URI']

# Route request to correct page script handler
try:
    route_dest = Route.getDestination (REQUEST_METHOD, URL_PATH)
    is_view = False
    view_file = ""
    controller_file = ""

    if type (route_dest) is tuple:
        if route_dest[0] == "view":
            is_view = True
            view_file = route_dest[1]
        else:
            controller_file = route_dest[1]
    else:
        controller_file = route_dest


    if is_view:
        # Render HTML
        view = View ("views/" + view_file)
        view.render()
    else:
        # ADD TO THIS
        util.print_html_header()
        print ("Need to finish controller launching code for application.")

    # Debug, remove in final version
    util.print_debug_version_and_env()

# If routing for a particular HTTP method and URL was not found, print 404 error
except UndefinedRouteError:
    print_error()

# form = cgi.FieldStorage()

# # Get data from fields
# first_name = form.getvalue('first_name')
# last_name  = form.getvalue('last_name')


# if environ.has_key('HTTP_COOKIE'):
#    for cookie in map(strip, split(environ['HTTP_COOKIE'], ';')):
#       (key, value ) = split(cookie, '=');
#       if key == "UserID":
#          user_id = value

#       if key == "Password":
#          password = value


# Parse URL

# QUERY_STRING # URL information

# REQUEST_METHOD # HTTP Method GET or POST
# CONTENT_LENGTH # Length of data sent in POST request

# # Client info
# HTTP_USER_AGENT # User agent of client
# REMOTE_ADDR # Remote address of client making request
# HTTP_COOKIE # Set cookies for the client


# HTTP Header: "Location: URL" for redirecting if not authenticated
# HTTP Header: "Set-Cookie: String" for setting cookies


# Check route map and run code for route


# Check url & token
# If authenticated:
#   show content
# Else display unauthorized or redirect to login
