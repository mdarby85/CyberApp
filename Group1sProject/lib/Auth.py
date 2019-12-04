#!/usr/bin/env python

from lib.Middleware import Middleware
from lib.Response import Response
from lib.Cookies import Cookies
from lib.User import User
from lib.Debug import Debug
from lib.config import *
import secrets
import base64
import hashlib
import os
import psycopg2  # Postgres Connection


class Auth (Middleware):

    # Override of middleware run method
    @staticmethod
    def run (environ, cookies):
        """
        Checks if user is authenticated. If so, returns none, otherwise redirects
        to login.

        @return Response of None
        """
        # If not authrorized, redirect, otherwize do nothing, continue route
        response = Response.redirect ('/login')
        if Auth.is_authorized (cookies):
            response = None

        return response


    @staticmethod
    def is_authorized (cookies):
        """
        Checks if the user is authorized to use the website based on their stored
        login cookie and the state of the database.
        """

        does_valid_token_exist = False

        auth_token_bytes = base64.b64decode (cookies.get (User.AUTH_TOKEN_COOKIE_KEY).encode (ENCODING))
        user_email = cookies.get (User.EMAIL_COOKIE_KEY)

        with psycopg2.connect (POSTGRES_DB_CONNECT) as conn:
            with conn.cursor() as curs:
                curs.execute ("SELECT does_user_auth_token_exist (%s, %s)",
                              (user_email, auth_token_bytes))
                does_valid_token_exist = curs.fetchone()[0]

        return does_valid_token_exist



    @staticmethod
    def attempt_login (user_email, user_password, cookies):
        """
        Attempts a user login.

        @return true if the user gets successfully logged in, false otherwise
        """
        login_attempt_success = False

        # Connect to DB in a singular transaction
        with psycopg2.connect (POSTGRES_DB_CONNECT) as conn:
            with conn.cursor() as curs:
                try:
                    curs.execute ("SELECT get_user_password_hash (%s)", (user_email,))
                    db_password_hash = curs.fetchone()

                    if db_password_hash is None or db_password_hash[0] is None:
                        return False

                    db_password_hash = db_password_hash[0].tobytes()

                    if not Auth.verify_password_hash (user_password, db_password_hash):
                        return False

                    # Generate authentication token
                    auth_token_bytes = secrets.token_bytes (SECURE_TOKEN_NUM_BYTES)
                    auth_token_string = base64.b64encode (auth_token_bytes).decode (ENCODING)

                    curs.execute ("CALL set_user_auth_token (%s, %s)",
                                  (user_email, auth_token_bytes))

                    # Store user data in cookies
                    cookies.set (User.EMAIL_COOKIE_KEY, user_email)
                    cookies.set (User.AUTH_TOKEN_COOKIE_KEY, auth_token_string)

                    login_attempt_success = True

                except psycopg2.Error:
                    pass

        # ? conn.close()  # Sucks but the docs say you have to do this??

        return login_attempt_success


    @staticmethod
    def hash_password (password_str):
        """
        Expects a str password and returns a hashed byte string.
        """
        assert (isinstance (password_str, str))

        if len (password_str) > MAX_PASSWORD_STR_LENGTH:
            password_str = password_str[:MAX_PASSWORD_STR_LENGTH]

        salt = os.urandom (HASH_SALT_NUM_BYTES)
        key = hashlib.pbkdf2_hmac (HASH_ALGORITHM,
                                   password_str.encode (encoding=ENCODING),
                                   salt,
                                   HASH_ITERATIONS)
        return salt + key


    @staticmethod
    def verify_password_hash (password_to_verify_str, true_hash_bytes):
        """
        Verifies that a str password hashes to a byte string.
        """
        assert (isinstance (password_to_verify_str, str))
        assert (isinstance (true_hash_bytes, bytes))

        if len (password_to_verify_str) > MAX_PASSWORD_STR_LENGTH:
            password_to_verify_str = password_to_verify_str[:MAX_PASSWORD_STR_LENGTH]


        true_hash_salt = true_hash_bytes[:HASH_SALT_NUM_BYTES]
        true_hash_key = true_hash_bytes[HASH_SALT_NUM_BYTES:]

        key_to_verify = hashlib.pbkdf2_hmac (HASH_ALGORITHM,
                                             password_to_verify_str.encode (encoding=ENCODING),
                                             true_hash_salt,
                                             HASH_ITERATIONS)

        return true_hash_key == key_to_verify
