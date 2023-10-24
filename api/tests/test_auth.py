#!/usr/bin/env python3
"""Test auth features.
"""
import json


users = [
    {'email': 'johndoe@gmail.com', 'firstname': 'John',
     'lastname': 'Doe', 'password': '123456'},
    {'email': 'janedoe@gmail.com', 'firstname': 'Jane',
     'lastname': 'Doe', 'password': '123456'},
]


def test_signin_signup(client, app):
    """
    """
    with app.app_context():
        for user in users:
            # Login a unknown user with the right creds.
            res = client.post(
                '/api/v1/auth/login',
                json={'email': user['email'], 'password': user['password']}
            )
            assert res.status_code == 401
            assert res.json['status'] == 'Unauthorized'

            # Signup a new user.
            res = client.post(
                '/api/v1/auth/signup',
                json={**user, 'confirmPassword': user['password']}
            )
            assert res.status_code == 200
            assert res.json['status'] == "OK"
            assert type(res.json['access_token']) is str

            # Signup an existing user.
            res = client.post(
                '/api/v1/auth/signup',
                json={**user, 'confirmPassword': user['password']}
            )
            assert res.status_code == 400
            assert res.json['status'] == "Bad Request"
            assert res.json['errors']['email'][0] == "Email already in use."

            # Signin an existing user with the right credentials.
            res = client.post(
                '/api/v1/auth/login',
                json={'email': user['email'], 'password': user['password']}
            )
            assert res.status_code == 200
            assert res.json['status'] == "OK"
            assert type(res.json['access_token']) is str

            # Signin an existing user with the wrong credentials.
            res = client.post(
                '/api/v1/auth/login',
                json={'email': user['email'], 'password': 'wrong password'}
            )
            assert res.status_code == 401
            assert res.json['status'] == 'Unauthorized'
