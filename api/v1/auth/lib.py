#!/usr/bin/python3
"""Auth module.

Contains utility functions related
to authentification and authorization.
"""
import bcrypt
from datetime import datetime, timedelta
from flask import jsonify, request
from functools import wraps
import jwt
import os


def hash_password(pswd):
    """Hashes a password using sha256 algo and return the hash.

    Arg:
        pswd (str): The password to hash.
    Returns:
        str: The hash as a hex string.
    """
    salt = bcrypt.gensalt()
    return bcrypt.hashpw(bytes(pswd, "utf-8"), salt)


def password_match(pswd, hashed_pswd):
    """Verifies if a password macthes the given hash.

    Args:
        pswd (str): The plain password.
        hashed_pswd (str): The hash of the password to test
                           against.
    Returns:
        (bool): True if the password is correct otherwise False.
    """
    return bcrypt.checkpw(pswd.encode('utf-8'), hashed_pswd)


def auth_required(func):
    """Jwt auth Decorator.

    Checks if correct Jwt credentials are included in the
    request headers (as Bearer token).

    Returns a 401 response if credentials are missing or invalids.

    If valid credentials are found, the decorated function is executed
    with the user's id passed as the first argument.
    """
    @wraps(func)
    def wrapper_auth_required(*args, **kwargs):
        token = request.headers.get('Authorization', None)
        if token is None:
            return jsonify({'status': 'unauthorized',
                            'message': 'Missing credentials'}), 401
        try:
            jwt_token = token.split('Bearer ')[1]
            payload = jwt.decode(jwt_token,
                                 os.getenv('JWT_SECRET'),
                                 algorithms=["HS256"])
            user_id = payload.get('sub')
        except Exception as e:
            return jsonify({'status': 'unauthorized',
                            'message': 'Invalid credentials'}), 401
        return func(user_id, *args, **kwargs)
    return wrapper_auth_required


def auth_silent(func):
    """Silently Checks if correct Jwt credentials are passed
    in the request headers.
    """
    @wraps(func)
    def wrapper_auth_silent(*args, **kwargs):
        token = request.headers.get('Authorization', None)
        if token is None:
            return func(None, *args, *kwargs)

        try:
            jwt_token = token.split('Bearer ')[1]
            payload = jwt.decode(jwt_token,
                                 os.getenv('JWT_SECRET'),
                                 algorithms=["HS256"])
            user_id = payload.get('sub')

            return func(user_id, *args, **kwargs)
        except Exception as e:
            return func(None, *args, *kwargs)
    return wrapper_auth_silent


def create_token(user_id):
    """
        Create and returns a JSON Web Token for the given user's id.

        Args:
            user_id(str): The user's id to create the token for.

        Returns:
            str: The base64 encoded token.
    """
    payload = {'sub': user_id,
               'exp': datetime.utcnow() + timedelta(hours=24)}
    token = jwt.encode(payload, os.getenv('JWT_SECRET'), algorithm="HS256")

    return token
