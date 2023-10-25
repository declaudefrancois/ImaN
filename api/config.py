#!/usr/bin/env python3
"""Configuration module.
"""
from dotenv import dotenv_values, load_dotenv
import os
from pydantic import BaseModel, UrlConstraints, RedisDsn, AmqpDsn
from pydantic.networks import MultiHostUrl
from pydantic.functional_serializers import PlainSerializer
from typing_extensions import Annotated


RedisDSN = Annotated[
    RedisDsn,
    PlainSerializer(lambda x: "{}".format(x),
                    return_type=str,
                    when_used="always")
]

MongoDSN = Annotated[
    MultiHostUrl,
    UrlConstraints(
        allowed_schemes=["mongodb", "mongodb+srv"],
    ),
    PlainSerializer(lambda x: "{}".format(x),
                    return_type=str,
                    when_used="always")
]


class ConfigSchema(BaseModel):
    """Configuration validation schema.
    """
    PORT: int
    JWT_SECRET: str
    MONGO_URI: MongoDSN
    MONGO_DBNAME: str
    CELERY_BROKER_URL: RedisDSN
    CELERY_BACKEND_URL: MongoDSN
    MAX_CONTENT_LENGTH: int = 20 * 1000 * 1000  # 20MB


def loadConfigFromEnvFile():
    """Loads the Configuration from .env file.
    """
    load_dotenv()

    config = {
        **dotenv_values(".env"),
        **os.environ,
    }
    if 'PORT' not in config:
        config['PORT'] = 3000

    return config
