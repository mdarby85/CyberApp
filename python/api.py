from webob import Request, Response
import hashlib
from wsgiref.simple_server import make_server


# Main class and functions where we define everything
# for our custom API implementation
class API:

    def __init__(self):
        self.routes = {}

    # Override the API class __call__ method
    def __call__(self, environ, start_response):
        request = Request(environ)
        # print(request) # prints request method i.e. GET, POST, etc

        response = self.handle_request(request)

        return response(environ, start_response)

    # This function is the default call when 
    # pages that do not exist are accessed
    def default_response(self, response):
        response.status_code = 404
        response.text = str(response.status_code) + " Page Not found!"

    # Main handler for handling path requests
    # Calls find_handler() to get path and 
    # handles it accordingly
    def handle_request(self, request):
        response = Response()

        handler = self.find_handler(request_path=request.path)

        # If handler is valid
        if handler is not None:
            handler(request, response)
        # Otherwise, return the default response
        else:
            self.default_response(response)
        return response

    # Checks if request_path is a valid route
    def find_handler(self, request_path):
        for path, handler in self.routes.items():
            if path == request_path:
                return handler

    # Returns the path
    def route(self, path):
        def wrapper(handler):
            self.routes[path] = handler
            return handler

        return wrapper

    def encrypt_string(self, hash_string):
        return hashlib.sha256(hash_string.encode()).hexdigest()
