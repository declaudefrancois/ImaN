#!/usr/bin/python3
"""Files blueprint for v1 Routes.

Contains the routes definitions for files processing
endpoints.
"""
from bson.objectid import ObjectId
from datetime import datetime
from flask import Blueprint, make_response, jsonify, abort, request, url_for
import math
import os
import pymongo
from schema import SchemaError
import uuid
from v1.images.schemas import ImageOperation
from v1.auth.lib import auth_silent, auth_required
from v1.images.lib import isValidImage, extension
from v1.images.tasks import process_image
from werkzeug.exceptions import NotFound
from werkzeug.utils import secure_filename


def get_images_bp(db):
    file_bp = Blueprint('images', __name__, url_prefix='/api/v1')

    @file_bp.route('/images', methods=['POST'])
    @auth_silent
    def process(user_id):
        """Starts a new processing job."""
        # Validate the request
        imageOperation = ImageOperation(**request.form)

        operation = imageOperation.model_dump()['operation']
        image = request.files['image'] if len(request.files) > 0 else None
        if image is None:
            return jsonify({'status': 'Bad Request',
                            'errors': 'Image is missing.'}), 400
        if not isValidImage(image):
            return jsonify({
                'status': 'Bad Request',
                'errors': 'Image must be a png or a jpeg file.'}), 400

        filename = "image-{}.{}".format(uuid.uuid4(),
                                        extension(image.filename))
        dest = os.path.join(os.getcwd(), 'uploads', filename)
        image.save(dest)

        user_object_id = ObjectId(user_id) if user_id is not None else None
        res = db['images'].insert_one({'user_id': user_object_id,
                                       'filename': filename,
                                       'operation': operation,
                                       'created_at': datetime.utcnow()})

        image_id = "{}".format(res.inserted_id)
        process_image.apply_async(
            ({'image_path': dest, 'operation': operation,
              'image_id': image_id},),
            retry=True,
            retry_policy={'max_retries': 3}
        )

        cb_url = url_for('images.job_status',
                         image_id=image_id,
                         _external=True)
        return jsonify({'status': 'OK', 'state': 'ACCEPTED',
                        'callback_url': cb_url}), 201

    @file_bp.route('/images/<image_id>/status', methods=['GET'])
    def job_status(image_id):
        """Get the status of the job.
        """
        row = db['images'].find_one({'_id': ObjectId(image_id)})
        if row is None:
            raise NotFound("Task not found")

        res_body = {'status': 'OK', 'state': row['status']}
        if row['status'] == 'SUCCESS':
            res_body['url'] = url_for(
                'static',
                filename=row['filename'],
                _external=True
            )

        return jsonify(res_body), 200

    @file_bp.route('/users/<user_id>/images', methods=['GET'])
    @auth_required
    def user_images(sub, user_id):
        """List a given user files.
        """
        user = db['users'].find_one({'_id': ObjectId(user_id)})
        if user_id != sub or user is None:
            return jsonify({'status': 'Unauthorized'}), 401

        page = int(request.args.get('page', '1'))
        limit = int(request.args.get('limit', '5'))

        images = db['images'].find(
            {'user_id': user['_id']},
            skip=limit * (page - 1),
            limit=limit,
            sort=[('created_at', pymongo.DESCENDING)]
        )
        count = db['images'].count_documents({'user_id': user['_id']})
        total_pages = math.ceil(count / limit)
        has_more = page < total_pages

        images = [{**x,
                   '_id': str(x['_id']),
                   'user_id': str(x['user_id']),
                   'created_at': x['created_at'].isoformat()} for x in images]
        return jsonify({'status': 'OK',
                        'data': images,
                        'page': page,
                        'total': count,
                        'total_pages': total_pages,
                        'has_more': has_more})

    return file_bp
