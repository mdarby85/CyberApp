#!/usr/bin/env python

from lib.Response import Response
from lib.Auth import Auth
from lib.View import View
from lib.config import *
from lib.User import User
import psycopg2  # Postgres Connection
import secrets


class LoginController:
    """
    Handles login page routes and functionality.
    """

    @staticmethod
    def get (request, cookies):
        # If already authorized, redirect to main page
        if Auth.is_authorized (cookies):
            return Response.redirect ('/')
        else:
            return Response.okDisplay (View ('views/login.html').get())


    @staticmethod
    def post_login (request, cookies):
        email = request.get ('email', [''])[0]  # Returns the first email value.
        password = request.get ('password', [''])[0]

        login_success = Auth.attempt_login (email, password, cookies)

        # If was not logged in, make them login again
        if not login_success:
            return Response.redirect ('/login')

        return Response.redirect ('/')


    @staticmethod
    def post_signup (request, cookies):
        email = request.get ('email', [''])[0]  # Returns the first email value.
        password = request.get ('password', [''])[0]

        if password is None or email is None:
            return Response.okDisplay ("Bad Parameters Sent!")

        if len (password) < MIN_PASSWORD_STR_LENGTH:
            return Response.okDisplay (View ('views/signup-error-password.html').get())

        password_hash = Auth.hash_password (password)
        email_verification_token_bytes = secrets.token_bytes (SECURE_TOKEN_NUM_BYTES)

        with psycopg2.connect (POSTGRES_DB_CONNECT) as conn:
            with conn.cursor() as curs:
                try:
                    curs.execute ("CALL create_user (%s, %s, %s)",
                                  (email, password_hash, email_verification_token_bytes))
                    # TODO: SEND VERIFICATION EMAIL HERE
                except psycopg2.Error:
                    # Debug.print (str(e))
                    pass  # Continue regardless of signup error

        # Regardless of actual user creation, we always report the same page so nobody can tell
        # what emails have accounts.
        return Response.okDisplay (View ('views/signup-success.html').get().format(user_email=email))


    @staticmethod
    def post_logout (request, cookies):
        # Clear Cookies
        cookies.set (User.EMAIL_COOKIE_KEY, '')
        cookies.set (User.AUTH_TOKEN_COOKIE_KEY, '')
        return Response.redirect ('/login')
