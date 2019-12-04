#!/usr/bin/env python

from http import cookies

HTTP_COOKIE_HEADER = 'Set-Cookie'

class Cookies:
    """
    An object allowing for getting and setting cookies. Every HTTP request will
    be paired with should call init_session_cookies to grab the currently
    available cookies and every HTTP reply should send any cookies added to this
    singleton class.
    """

    def __init__ (self, environ):
        self.cookies_kv = cookies.SimpleCookie()
        self.cookies_kv.load (environ.get ('HTTP_COOKIE', ''))
        self.has_cookie_changed = False


    def get (self, cookie_key):
        """
        @return string cookie value for key or empty string if key not found
        """
        morsel = self.cookies_kv.get (cookie_key)
        return "" if morsel is None else morsel.value


    def set (self, key, value):
        """
        Sets a cookie key and value.
        """
        if self.cookies_kv.get(key) is None or value != self.cookies_kv[key]:
            self.has_cookie_changed = True
            self.cookies_kv[key] = value


    def getAll (self):
        return [(HTTP_COOKIE_HEADER, item.strip()) for item in self.cookies_kv.output(header='').splitlines()]
