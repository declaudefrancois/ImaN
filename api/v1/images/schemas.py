#!/usr/bin/env python3
"""Schemas validation for images requests.
"""
from enum import Enum
from pydantic import BaseModel
from werkzeug.datastructures import FileStorage
from typing_extensions import Annotated, TypeAliasType


class OperationType(str, Enum):
    compress = 'compress'
    toJPEG = 'toJPEG'
    toPNG = 'toPNG'


class ImageOperation(BaseModel):
    operation: OperationType = OperationType.compress
