#!/usr/bin/env python3
"""Configuration module.
"""
from dotenv import dotenv_values, load_dotenv
import os
from pydantic import BaseModel


class ConfigSchema(BaseModel):
    """Configuration validation schema.
    """
    PORT: int
    JWT_SECRET: str
    MONGO_URI: str
    MONGO_DBNAME: str


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
