#!/usr/bin/env python3
"""
    Instantiates the main app and starts
    up the web server.
"""
from app import init_app


# Init the app here so we can access it when binding gunicorn.
app = init_app()

if __name__ == "__main__":
    # Start the applicationi web server.
    app.run(host='0.0.0.0', port=app.config['PORT'], threaded=True)
