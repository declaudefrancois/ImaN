#!/usr/bin/env python3
"""
    Instantiates the main app and starts
    up the web server.
"""
from app import init_app


if __name__ == "__main__":
    # Init & start the applicationi web server.
    app = init_app()
    app.run(host='0.0.0.0', port=app.config['PORT'], threaded=True)
