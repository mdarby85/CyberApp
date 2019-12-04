#!/usr/bin/env python

"""
A View is an HTML rendering of a page.
"""


class View:

    html_file = ""
    html_string = ""

    def __init__ (self, html_file):
        self.html_file = html_file
        with open (self.html_file) as f:
            self.html_string = f.read()

    def get (self):
        return self.html_string
