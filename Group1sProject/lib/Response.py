#!/usr/bin/env python

HTTP_STATUS_OK = '200 OK'
HTTP_STATUS_REDIRECT = '303 See Other'
HTTP_STATUS_NOT_FOUND = '404 Not Found'


class Response:
    """
    An HTTP Response to the client.
    """

    def __init__ (self, status_http, output_html, headers_http=[]):
        assert (isinstance (status_http, str))
        assert (isinstance (output_html, str))
        assert (isinstance (headers_http, list))
        for header in headers_http:
            assert (isinstance (header, tuple))
            for item in header:
                assert (isinstance (item, str))

        self.status_http = status_http    # str
        self.output_html = output_html    # str
        self.headers_http = headers_http  # list

    @staticmethod
    def okDisplay (output_html):
        """
        Factory method to construct a Response for an HTTP OK status page display.
        """
        assert (isinstance (output_html, str))
        return Response (HTTP_STATUS_OK, output_html)

    @staticmethod
    def redirect (url_path):
        """
        Factory method to construct a Response for a redirect.
        """
        assert (isinstance (url_path, str))
        return Response (HTTP_STATUS_REDIRECT, "", [('Location', url_path)])

    @staticmethod
    def notFound404Error():
        """
        Factory method to construct a Response for a 404 error.
        """
        return Response (HTTP_STATUS_NOT_FOUND,
                         '<h1>404 Error : Some crap went down, so just <a href="/">HEAD HOME</a>.</h1>',
                         [])
