#!/usr/bin/env python3
"""Mongodb integration.
"""
from os import getenv
from pymongo import MongoClient


mongo_client = None
db = None


def get_db():
    """Get the db connection.
    """
    global db
    return db


def get_db_client():
    """Get the db client.
    """
    global mongo_client
    return mongo_client


def init_db_connection(uri, db_name):
    """Initiates the connection to mongoDb.

    Sets the globals `mongo_client` and `db` to the created
    client and database objects.

    Args:
        uri (str): The uri of the mongoDb server.
        db_name (str): The database to use.
    """
    global mongo_client
    global db

    if mongo_client is not None:
        return
    print(">>>> connecting to {} in database {}".format(uri, db_name))
    mongo_client = MongoClient(uri)
    db = mongo_client[db_name]
    print(">>>> connection successfully established")

    return db


def close_db_connection():
    """Close the connection to mongoDb.
    """
    global mongo_client
    global db

    if mongo_client is None:
        return

    print(">>>> closing connection to mongoDB.")
    mongo_client.close()
    mongo_client = None
    db = None
    print(">>>> Bye...")
