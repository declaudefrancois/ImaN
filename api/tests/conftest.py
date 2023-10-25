#!/usr/bin/env python3
"""App fixtures.
"""
import pytest
from app import init_app
from db import get_db
import os


@pytest.fixture()
def app():
    os.environ["JWT_SECRET"] = 'secret'
    os.environ["MONGO_DBNAME"] = 'iman_test'
    os.environ["MONGO_URI"] = 'mongodb://localhost:27017'
    os.environ["CELERY_BROKER_URL"] = 'redis://localhost'
    os.environ["CELERY_BACKEND_URL"] = 'mongodb://localhost:27017'

    app = init_app()
    app.config.update({
        "TESTING": True,
    })

    db = get_db()
    for collection in db.list_collections():
        db.drop_collection(collection['name'])

    yield app

    db = get_db()
    for collection in db.list_collections():
        db.drop_collection(collection['name'])


@pytest.fixture()
def client(app):
    return app.test_client()


@pytest.fixture()
def runner(app):
    return app.test_cli_runner()
