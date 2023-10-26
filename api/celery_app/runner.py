#!/usr/bin/env python3
"""Inits and start the celery job runner process.
"""
from app import init_app


flask_app = init_app()
celery_app = flask_app.extensions["celery"]
