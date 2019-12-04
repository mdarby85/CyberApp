#!/usr/bin/env python

"""
A Route is a URL path mapping to an HTTP action (get, post),
and possibly a Controller. This is a singleton class that holds
the routing map which is intended to be defined in routes.py.
"""

import types


class UndefinedRouteError (Exception):
    """
    Exception raised when a route is attempted to be accessed from the route_map
    but that route is not present.
    """
    pass

    def __init__ (self, http_method, url_path):
        self.http_method = http_method
        self.url_path = url_path



class Route:
    HTTP_GET = 'GET'
    HTTP_POST = 'POST'

    route_map = {
        HTTP_GET: {},
        HTTP_POST: {}
    }

    @classmethod
    def getDestination (cls, http_method, url_path):
        """
        Returns a route destination from the route_map given the http_method and
        the url_path.

        @param http_method "get" or "post"
        """
        if http_method in cls.route_map and url_path in cls.route_map[http_method]:
            return cls.route_map[http_method][url_path]
        else:
            raise UndefinedRouteError (http_method, url_path)


    @classmethod
    def get (cls, url_path, controller_cmd, middleware_cmd_list=[]):
        """
        Defines an HTTP GET route at the specified `url_path` to run the
        specified `controller` code to display some dynamic View to the user.
        """
        cls.assertControllerType (controller_cmd)
        cls.assertMiddlewareTypes (middleware_cmd_list)
        cls.route_map[cls.HTTP_GET][url_path] = (controller_cmd, middleware_cmd_list)


    @classmethod
    def view (cls, url_path, view_file, middleware_cmd_list=[]):
        """
        A shortcut method for get() which also uses an HTTP GET action but
        bypasses a controller and goes straight to displaying a view. This is
        useful for any static pages in the site.
        """
        cls.assertViewType (view_file)
        cls.assertMiddlewareTypes (middleware_cmd_list)
        cls.route_map[cls.HTTP_GET][url_path] = (view_file, middleware_cmd_list)


    @classmethod
    def post (cls, url_path, controller_cmd, middleware_cmd_list=[]):
        """
        Defines an HTTP POST action at the specified `url_path` to run the
        specified `controller` code to process POSTed user data.
        """
        cls.assertControllerType (controller_cmd)
        cls.assertMiddlewareTypes (middleware_cmd_list)
        cls.route_map[cls.HTTP_POST][url_path] = (controller_cmd, middleware_cmd_list)


    # Private Assert Methods ===================================================

    @classmethod
    def assertControllerType (cls, controller_cmd):
        assert (isinstance (controller_cmd, types.FunctionType))

    @classmethod
    def assertMiddlewareTypes (cls, middleware_cmd_list):
        for middleware_cmd in middleware_cmd_list:
            assert (isinstance (middleware_cmd, types.FunctionType))

    @classmethod
    def assertViewType (cls, view_file):
        assert (isinstance (view_file, str))
