from lib.Route import *
from lib.Auth import *
from controllers.LoginController import *
from controllers.IndexController import *
from controllers.FriendController import *

# Homepage / User Management
Route.get ("/", IndexController.get, [Auth.run])

# User Management Post URLS
Route.post ("/friend/add", FriendController.post_add_friend, [Auth.run])
Route.post ("/friend/remove", FriendController.post_remove_friend, [Auth.run])

# Login / Signup
Route.get ("/login", LoginController.get)
Route.post ("/login", LoginController.post_login)
Route.post ("/signup", LoginController.post_signup)
Route.post ("/logout", LoginController.post_logout)
