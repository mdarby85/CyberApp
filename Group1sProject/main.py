#!/usr/bin/env python

"""
Main web app user management server functionality.
This is meant to be used with the Apache module mod_wsgi to run.

For testing launch with:
    mod_wsgi-express start-server <THIS FILE'S NAME>
"""

from lib.Route import Route   # Our custom framework library
from lib.View import View    # Our custom framework library
from lib.Response import *
from lib.Cookies import Cookies
from lib.config import *
from lib.Debug import Debug
from routes import *    # Website routes
from config import *
from cgi import parse_qs, escape

# if CONF_DEBUG:
#   import cgitb  # CGI Traceback Manager in Browser which doesnt actually work :/
#   cgitb.enable()


def application (environ, start_response):
    """
    Main WSGI Application
    Based on input from `environ` about the HTTP request, we react by attempting
    to route the request via the `attemptRoute` function. We then output the
    associated HTTP Headers and return the content as a byte list.

    @param enciron: environment variables such as request method
    @param start_response: method to output HTTP status and headers
    """
    # Debug.reset()
    response = attemptRoute (environ)

    output_bytes = response.output_html.encode (encoding=ENCODING, errors='strict')


    response_headers = [('Content-type', 'text/html'),
                        ('Content-Length', str (len (output_bytes)))]

    response_headers.extend (response.headers_http)

    # Send HTTP Headers
    start_response (response.status_http, response_headers)

    # Send Page Content as byte list
    return [output_bytes]


def attemptRoute (environ):
    """
    Attempts running a route based on environ input of HTTP request content.

    @return Response: an HTTP response
    """
    REQUEST_METHOD = environ['REQUEST_METHOD']
    REQUEST_URI = environ['REQUEST_URI']
    cookies = Cookies (environ)

    response = None

    # Route request to correct page script handler
    try:
        route_dest = Route.getDestination (REQUEST_METHOD, REQUEST_URI)
        is_view = False
        view_file = ''
        controller_cmd = ''
        middleware_cmd_list = []

        assert (isinstance (route_dest, tuple))

        if isinstance (route_dest[0], str):
            is_view = True
            view_file = route_dest[0]
        else:
            controller_cmd = route_dest[0]

        middleware_cmd_list = route_dest[1]

        # Run middleware before main route
        for middleware_cmd in middleware_cmd_list:
            middleware_response = middleware_cmd (environ, cookies)
            if middleware_response is not None:
                response = middleware_response
                break

        if response is None:
            # Return view or run controller to produce HTTP response with HTML
            if is_view:
                response = Response.okDisplay (View (f"views/{view_file}").get())
            else:  # Run controller file
                response = controller_cmd (parse_request (environ), cookies)

    except UndefinedRouteError:
        response = Response.notFound404Error()

    finally:
        if response is None:
            response = Response.okDisplay ("ERROR")

        if response is not None:
            # Debug Output
            if CONF_DEBUG:
                response.output_html = f'{response.output_html}\n\n\n{get_debug_string (environ, cookies, response)}'

            if cookies.has_cookie_changed:
                response.headers_http.extend (cookies.getAll())

            # Replace any cookies
            # resp_cookie_index = 0
            # for resp_header, resp_cookie in response.headers_http:
            #     if resp_header == 'Set-Cookie':
            #         resp_cookie_list = resp_cookie.split ('=', 1)
            #         resp_cookie_key = resp_cookie_list[0]
            #         resp_cookie_val = resp_cookie_list[1]
            #         for local_cookie_morsel in cookies.cookies_kv.values():
            #             if local_cookie_morsel.key == resp_cookie_key:
            #                 if local_cookie_morsel.value != resp_cookie_val:
            #                     response.headers_http[resp_cookie_index] = f'{local_cookie_morsel.key}={local_cookie_morsel.value}'
            #     resp_cookie_index += 1


            # # Add any new cookies to the response
            # for cookie in cookies.getAll():
            #     if cookie not in response.headers_http:
            #         response.headers_http.append (cookie)

        return response


def parse_request (environ):
    """
    Parses user post input of the HTTP request.
    """
    # environ variable CONTENT_LENGTH may be empty or missing
    try:
        request_body_size = int (environ.get ('CONTENT_LENGTH', 0))
    except (ValueError):
        request_body_size = 0

    # When the method is POST the variable will be sent
    # in the HTTP request body which is passed by the WSGI server
    # in the file like wsgi.input environment variable.
    request_body = environ['wsgi.input'].read (request_body_size)
    request_dict = parse_qs (request_body.decode())

    # Escape user input except for 'password fields'
    for key, valList in request_dict.items():
        if key != 'password':
            for val in valList:
                val = escape (val)

    return request_dict


def get_debug_string (environ, cookies, response):
    return (f"<h1>DEBUG</h1>\n"
            f"<h2>User Debug Printing:</h2>"
            f"{Debug.debug_string}\n"
            f"<h2>HTTP Response:</h2>\n"
            f"<ul>\n"
            f"<li><strong>HTTP Status:</strong> {response.status_http}</li>\n"
            f"<li><strong>HTTP Headers:</strong> {response.headers_http}</li>\n"
            f"</ul>\n"
            f"<h2>Cookies:</h2>\n"
            f"{cookies.getAll()}"
            f"<h2>Environ:</h2>\n"
            f"{environ}\n")
