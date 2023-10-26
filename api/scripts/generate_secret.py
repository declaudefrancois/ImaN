#!/usr/bin/env python3
"""Generates a secret string for jwt tokens.
"""
from secrets import token_urlsafe


print(token_urlsafe(256))
