# pylint: disable=invalid-name
""" Paths """
from pathlib import Path
import secrets as s

PROJECT_ROOT = Path(__file__).resolve().parent.parent
RECORDINGS_DIR = PROJECT_ROOT / "recordings"
RECORDINGS_DIR.mkdir(exist_ok=True)

RECORDING_FILE = "received.txt"
RECORDING_FILE_OLD = "~" + RECORDING_FILE

RECORDING_PATH = RECORDINGS_DIR / RECORDING_FILE
RECORDING_PATH_OLD = RECORDINGS_DIR / RECORDING_FILE_OLD

# Web page file (not used in this iteration)
WEB_DIR = Path(__file__).parent/"web"

PORT = s.PORT
WIO_URL = f"http://{s.WIO_NODE}:{s.PORT}/"
