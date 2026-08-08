# pylint: disable=invalid-name
# pylint: disable=global-statement
""" Python Server """
from pathlib import Path
from http.server import BaseHTTPRequestHandler, HTTPServer
import socket

import gui
import paths as p

CURRENT = ""
BACKUP = ""

def _rotate_file():
    BACKUP.unlink(missing_ok=True)

    if CURRENT.exists():
        CURRENT.rename(BACKUP)

class RequestHandler(BaseHTTPRequestHandler):
    """ HTTP Request Handler """
    def _not_found(self, filename):
        self.send_response(404)
        self.end_headers()
        self.wfile.write(f"{filename} not found".encode("utf-8"))

    def do_POST(self):
        """ Handle POST request """
        global CURRENT, BACKUP

        content_length = int(self.headers.get("Content-Length", 0))

        match self.path:
            case "/recorder/done":
                post_content = self.rfile.read(content_length).decode("utf-8")

                if post_content:
                    print("POST body:")
                    print(f"{post_content!r}")

                gui.mark_recording_finished()
                response = b"Button reset"

            case "/file/send":
                # rotate files
                CURRENT = p.RECORDING_PATH
                BACKUP = p.RECORDING_PATH_BACKUP

                _rotate_file()

                # Read the file a chunk at a time
                remaining = content_length

                with CURRENT.open("wb") as f:
                    while remaining > 0:
                        chunk = self.rfile.read(min(5, remaining))

                        if not chunk:
                            raise ConnectionError(
                                f"Upload ended with {remaining} bytes remaining"
                            )

                        f.write(chunk)
                        remaining -= len(chunk)

                response = b"File received"

            case _:
                self._not_found(self.path)
                return

        self.send_response(200)
        self.send_header("Content-Type", "text/plain")
        self.send_header("Content-Length", str(len(response)))

        self.end_headers()
        self.wfile.write(response)

def run_server():
    """ Run the server """
    server = HTTPServer(("0.0.0.0", p.PORT), RequestHandler)
    hostname = socket.gethostname()
    hostaddr = socket.gethostbyname(hostname)
    print(f"Starting server at http://{hostname}:{str(p.PORT)}/ ({hostaddr})")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
        print("Server stopped")
