#!/usr/bin/env python3
"""Configuration module.
"""
from dotenv import dotenv_values
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
    config = {
        **dotenv_values(".env"),
        **os.environ,
    }

    return config
