#!/usr/bin/env python3
"""Setup the application server.
"""
from config import ConfigSchema, loadConfigFromEnvFile
from db import init_db_connection
from celery_app.app import celery_init_app
from flask import Flask, make_response, jsonify
from flask_cors import CORS
from pydantic import ValidationError
from v1.auth import get_auth_bp
from v1.images import get_images_bp
from werkzeug.exceptions import HTTPException


def init_app():
    """Initalize the web application and return the app object.
    """
    app = Flask(__name__)
    cors = CORS(app, resources={r"/api/*": {"origins": "*"}})

    # Load and validate config.
    config = ConfigSchema(**loadConfigFromEnvFile())
    app.config.from_object(config)

    # Connect to mongoDB.
    db = init_db_connection(app.config['MONGO_URI'],
                            app.config['MONGO_DBNAME'])

    # Register app routes.
    app.register_blueprint(get_auth_bp(db))
    app.register_blueprint(get_images_bp(db))

    # Add the celery extension.
    celery_init_app(app)

    @app.route('/api/v1/status', methods=['GET'])
    def login():
        """Status Check endpoint."""
        return jsonify({'status': 'OK'})

    @app.errorhandler(HTTPException)
    def not_found(error):
        """ handles HTTPException. """
        return jsonify({'status': error.name}), error.code

    @app.errorhandler(ValidationError)
    def bad_request(error):
        """Bad requests handler

        Handle exception thrown whenever a Pydantic validation
        fail.

        Args:
            error (ValidationError): The exception thrown.
        Returns:
            tupple: The response and the status.
        """
        return jsonify({'status': 'Bad request',
                        'errors': error.errors()})

    @app.errorhandler(Exception)
    def server_error(e):
        """Handles all other unhundled exceptions."""
        print(e)
        return jsonify({'status': 'Internal Server Error'}), 500

    return app
