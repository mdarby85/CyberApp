#!/usr/bin/env python

from lib.Response import Response
from lib.View import View
from lib.Cookies import Cookies
from lib.User import User
from enum import Enum
import psycopg2  # Postgres Connection
from lib.config import *

# In DB:
# Friends Table =========
# uid firstUser
# uid secondUser
# firstUserStatus : accept or reject
# secondUserStatus : accept or reject
# timestamp of last update
#
# Only friends if both have accepted status. Default is reject status


class FriendStatus (Enum):
    unspecified = 0
    rejected = 1
    accepted = 2


class FriendController:
    """
    Handles any friend/user management requests.
    """

    @staticmethod
    def post_add_friend (request, cookies):
        """
        Adds a friend or accepts a friend request. A positive accept action is
        made by one user for another user.
        """
        this_user_email = cookies.get (User.EMAIL_COOKIE_KEY)
        related_user_email = request.get ('user_email', [''])[0]
        this_user_status = 'accepted'

        FriendController.update_friend_status (this_user_email, related_user_email, this_user_status)

        return Response.redirect ('/')


    @staticmethod
    def post_remove_friend (request, cookies):
        """
        Removes a friend or rejects a friend request. A negative reject action is
        made by one user for another user.
        """
        this_user_email = cookies.get (User.EMAIL_COOKIE_KEY)
        related_user_email = request.get ('user_email', [''])[0]
        this_user_status = 'rejected'

        FriendController.update_friend_status (this_user_email, related_user_email, this_user_status)

        return Response.redirect ('/')


    @staticmethod
    def update_friend_status (this_user_email, related_user_email, this_user_status):
        with psycopg2.connect (POSTGRES_DB_CONNECT) as conn:
            with conn.cursor() as curs:
                try:
                    curs.execute ("CALL set_user_friend_status (%s, %s, %s)",
                                  (this_user_email, related_user_email, this_user_status))
                except psycopg2.Error:
                    # Debug.print (str(e))
                    pass  # Continue regardless of friending error




