#!/usr/bin/env python3
"""Celery integration with flask.
"""
from celery import Celery, Task
from flask import Flask


def celery_init_app(app: Flask) -> Celery:
    class FlaskTask(Task):
        def __call__(self, *args: object, **kwargs: object) -> object:
            with app.app_context():
                return self.run(*args, **kwargs)

    celery_app = Celery(app.name, task_cls=FlaskTask)
    celery_app.conf.update(
        broker_url="{}".format(app.config['CELERY_BROKER_URL']),
        result_backend="{}".format(app.config['CELERY_BACKEND_URL']),
    )
    celery_app.set_default()
    app.extensions["celery"] = celery_app
    return celery_app
