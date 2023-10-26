#!/usr/bin/env python3
"""Celery tasks definitions for image processing.
"""
from bson.objectid import ObjectId
from celery import shared_task
from celery.utils.log import get_task_logger
from db import get_db
import os
from shutil import move


logger = get_task_logger(__name__)


@shared_task(bind=True, ignore_result=False)
def process_image(self, data):
    """Handles an image processing job.
    """
    logger.info(">>> processing task with data {}".format(data))
    db = get_db()

    # TODO: call the C processor here.

    # Move the processed image in the static folder or in a CDN in the future.
    dest = os.path.join(os.getcwd(), 'static')
    move(data['image_path'], dest)

    image_obj_id = ObjectId(data['image_id'])
    res = db['images'].find_one_and_update(
        {'_id': image_obj_id},
        {'$set': {'status': 'SUCCESS'}},
    )

    logger.info(">>>> DONE !!!")
    return 'SUCCESS'
