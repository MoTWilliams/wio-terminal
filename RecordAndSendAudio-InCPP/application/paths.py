# pylint: disable=invalid-name
""" Paths """
from pathlib import Path
import secrets as s

WEB_DIR = Path(__file__).parent/"web"

PORT = s.PORT
WIO_URL = f"http://{s.WIO_NODE}:{s.PORT}/"
