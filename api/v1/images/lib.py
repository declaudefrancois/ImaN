#!/usr/bin/env python3
"""Image utilities.
"""
import mimetypes
from werkzeug.datastructures import FileStorage


# TODO: read the file magic number to check the always exact mime-type.
def isValidImage(image: FileStorage) -> bool:
    """Checks if an image is valid.

    A valid image is either a PNG or a JPEG.
    These are the image format currently supported.

    Args:
        image (FileStorage): The uploaded image to check.
    Returns:
        bool: True if valid otherwise False.
    """
    ALLOWED_MIMES = ['image/jpeg', 'image/png']
    encoding, _ = mimetypes.guess_type(image.filename)
    return encoding in ALLOWED_MIMES


def extension(image):
    """Returns the extensions of the image.
    """
    return image.rsplit('.', 1)[1].lower()
