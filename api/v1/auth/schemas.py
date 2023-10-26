#!/usr/bin/env python3
from email_validator import validate_email
from pydantic import BaseModel, AfterValidator
from pydantic_core import PydanticCustomError
from typing_extensions import Annotated


def validateEmail(v):
    try:
        validate_email(v)
        return v
    except Exception:
        raise PydanticCustomError(
            'invalid_email',
            'value is not a valid email',
            dict(wrong_value=v),
        )


EmailType = Annotated[
    str,
    AfterValidator(validateEmail),
]


class LoginSchema(BaseModel):
    """Login request body validation schema.
    """
    email: EmailType
    password: str


class SignupSchema(BaseModel):
    """Signup request body validation schema.
    """
    email: EmailType
    firstname: str
    lastname: str
    password: str
    confirmPassword: str
