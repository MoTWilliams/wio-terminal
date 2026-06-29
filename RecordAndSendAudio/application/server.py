# pylint: disable=invalid-name
# pylint: disable=global-statement
""" Python Server """
from http.server import BaseHTTPRequestHandler, HTTPServer
import socket

import gui
import paths as p

post_content = ""

class RequestHandler(BaseHTTPRequestHandler):
    """ HTTP Request Handler """
    def _not_found(self, filename):
        self.send_response(404)
        self.end_headers()
        self.wfile.write(f"{filename} not found".encode("utf-8"))

    def do_POST(self):
        """ Handle POST request """
        global post_content

        content_length = int(self.headers.get("Content-Length", 0))
        post_content = self.rfile.read(content_length).decode("utf-8")
        print(post_content)

        match self.path:
            case "/recording/stopped":
                gui.mark_recording_finished()
                response = "Button reset".encode("utf-8")

                self.send_response(200)
                self.send_header("Content-Type", "text/plain")
                self.send_header("Content-Length", str(len(response)))

                self.end_headers()
                self.wfile.write(response)
            case _:
                self._not_found(self.path)

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
