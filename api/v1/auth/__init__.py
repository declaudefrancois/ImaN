#!/usr/bin/python3
"""Auth bluepprint for v1 Routes.

Contains the routes definitions of authentification
endpoints.
"""
from flask import Blueprint, make_response, jsonify, abort, request
from werkzeug.exceptions import HTTPException
from schema import SchemaError
from v1.auth.schemas import LoginSchema, SignupSchema
from v1.auth.lib import hash_password, create_token, password_match
from pydantic import ValidationError


def get_auth_bp(db):
    auth_bp = Blueprint('auth_bp', __name__, url_prefix='/api/v1/auth')

    @auth_bp.route('/login', methods=['POST'])
    def login():
        """Authenticates an existing"""
        # validate and sanitize data.
        data = LoginSchema(**request.get_json()).model_dump()

        user = db['users'].find_one({'email': data['email']})
        if user is None or not password_match(data['password'],
                                              user['password']):
            return jsonify({'status': 'Unauthorized'}), 401

        token = create_token(str(user['_id']))
        return jsonify({'status': 'OK', 'access_token': token})

    @auth_bp.route('/signup', methods=['POST', 'GET'])
    def signup():
        """Creates a new user"""
        # validate and sanitize data.
        data = SignupSchema(**request.get_json()).model_dump()

        if db['users'].find_one({'email': data['email']}) is not None:
            errors = {'email': ['Email already in use.']}
            return jsonify({'status': 'Bad Request', 'errors': errors}), 400

        data['password'] = hash_password(data['password'])
        res = db['users'].insert_one({'email': data['email'],
                                      'firstname': data['firstname'],
                                      'lastname': data['lastname'],
                                      'password': data['password']})

        user = db['users'].find_one({"_id": res.inserted_id})
        token = create_token(str(user['_id']))
        return jsonify({'status': 'OK', 'access_token': token})

    return auth_bp
