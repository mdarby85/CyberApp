# MapZest : User Management Web Application

This application allows users to sign up, log in, view other users' names,
make friend requests, accept friend requests, and remove friends.

It uses a Python Apache Integration called mod_wsgi. Reference these docs for
info on developing with mod_wsgi: https://modwsgi.readthedocs.io/en/develop/

## Development

### Local Dev
To get going with development on a local machine we will use the Python package
mod_wsgi to run Apache.

First, install mod_wsgi
```bash
pip install mod-wsgi
```

After installation, you now have access to the `mod_wsgi-express` program. Run
this to launch the test server:
```bash
mod_wsgi-express start-server
```
Now go to `http://localhost:8000/` in your browser and you should see a super
weird landing page with some snakes in a bottle of yellow liquid and something
about malt whiskey. Super weird defualt page, I know.

But, we want this Apache server to run our own Python CGI script, not this nasty
lookin landing page. To do this, run the following command in the
`/user_management_web_app` directory on our main.py file:
```bash
mod_wsgi-express start-server main.py
```

### Debugging
When starting `mod_wsgi-express` it lists where the error log file is. This may
be a file like: `/var/tmp/mod_wsgi-localhost:8000:501/error_log`. Go to that
location to see any errors that happen when "Internal Server Error" appears in
the HTML.

I have yet to find a successful way to print nice debug information to the browser
when an error occurs. (Obviously something like this would be removed in production).
But there are nice options like `cgitb` CGI traceback manager or Paste error middleware.
