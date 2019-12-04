#!/usr/bin/env python

from lib.Response import Response
from lib.View import View
from lib.User import *
from lib.Debug import Debug
from lib.Cookies import Cookies
import psycopg2  # Postgres Connection
from lib.config import *


class IndexController:
    """
    Manages homepage account display.
    """

    @staticmethod
    def get (request, cookies):

        user_email = cookies.get (User.EMAIL_COOKIE_KEY)
        friend_requests = []
        friend_list = {}
        potential_friend_list = []


        # Get related friend data from DB
        with psycopg2.connect (POSTGRES_DB_CONNECT) as conn:
            with conn.cursor() as curs:
                try:
                    # Friend Requests
                    curs.execute ("SELECT * FROM get_user_friend_requests (%s)", [user_email])
                    Debug.print (str(curs))
                    for record in curs:
                        if len (record) >= 2:
                            friend_requests.append (record[1])

                    # Friend List
                    curs.execute ("SELECT * FROM get_user_friends (%s)", [user_email])
                    Debug.print (str(curs))
                    for record in curs:
                        if len (record) >= 4:
                            friend_list[record[1]] = {
                                'latitude': record[2],
                                'longitude': record[3]
                            }

                    # Potential Friends
                    curs.execute ("SELECT * FROM get_user_potential_friends (%s)", [user_email])
                    for record in curs:
                        if len (record) >= 2:
                            potential_friend_list.append (record[1])

                except psycopg2.Error as e:
                    Debug.print (str(e))
                    pass  # Continue regardless of error


        # GET FRIEND REQUESTS FROM DB
        # friend_requests = ['timarterbury@gmail.com', 'yomamma@yomamma.com',
        #                    'Idunno@whatever.com', 'heyheyhey@mmmk.org']

        # # GET FRIEND LIST AND LOCATIONS FROM DB
        # friend_list = {
        #                 'joebob@qwert.poo':
        #                     {
        #                         'latitude': '8393.53324',
        #                         'longitude': '3920134.4'
        #                     },
        #                     'another@wut.omg':
        #                     {
        #                         'latitude': '74344.44',
        #                         'longitude': '32442.4'
        #                     },
        #                     'swagdank@wwww.com':
        #                     {
        #                         'latitude': '74344.44',
        #                         'longitude': '32442.4'
        #                     }
        #                 }

        # # GET USER LIST AND LOCATIONS FROM DB
        # user_list = ['wut', 'hey', 'omg', 'okay']

        friend_request_html = View ("views/friend-request.html").get()
        friend_list_html = View ("views/friend-list.html").get()
        potential_friend_list_html = View ("views/user-list.html").get()

        friend_list_html_filled = []
        for friend_email, location in friend_list.items():
            friend_list_html_filled.append (
                friend_list_html.format (user_email=friend_email,
                                         latitude=location['latitude'],
                                         longitude=location['longitude']))

        main_page = View (f"views/index.html").get()
        main_page = main_page.format (
            user_email=user_email,
            friend_requests=''.join (map (friend_request_html.format, friend_requests)),
            friend_list=''.join (friend_list_html_filled),
            potential_friend_list=''.join (map (potential_friend_list_html.format, potential_friend_list))
        )

        return Response.okDisplay (main_page)
