# pylint: disable=invalid-name
# pylint: disable=import-outside-toplevel
"""
Simple HTTP server in Python: 
https://gist.github.com/mdonkers/63e115cc0c79b4f6b8b3a6b797e485c7
"""
from http.server import BaseHTTPRequestHandler, HTTPServer
import logging
from requests import post # Install this in a virtual environment
import paths as p
import arduino_secrets as s

logging.basicConfig(
        level=logging.INFO,
        format="[%(asctime)s] %(levelname)s (%(name)s): %(message)s"
        )
server_log = logging.getLogger("server")
wio_log = logging.getLogger("wio")

WIO_URL = f"http://{s.WIO_NODE}:{s.PORT}/"

class RequestHandler(BaseHTTPRequestHandler):
    """ Custom HTTP request handler
    """
    def _not_found(self, filename):
        self.send_response(404)
        self.end_headers()
        self.wfile.write(f"{filename} not found".encode("utf-8"))

    def _send_file(self, filename, content_type):
        """ Send the file
        """
        file_path = p.WEB_DIR/filename

        if not file_path.exists():  # File not found
            self._not_found(filename)
            return

        data = file_path.read_bytes()

        self.send_response(200)     # Begin success message
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)      # Send the actual file

    def do_GET(self):
        """ Retrieve the webpage
        """
        match self.path:
            case "/":
                self._send_file("index.html", "text/html")
            case "/style.css":
                self._send_file("style.css", "test/css")
            case "/app.js":
                self._send_file("app.js", "application/javascript")
            case "/favicon.ico":
                self.send_response(204)
                self.end_headers()
            case "/.well-known/appspecific/com.chrome.devtools.json":
                self.send_response(204)
                self.end_headers()
            case _:
                self._not_found(self.path)

    def do_POST(self):
        """ Handle the BEEP request
        """
        match self.path:
            case "/api/beep":
                # Send beep POST request
                post(WIO_URL, data="beep", timeout=2)

                # Report success only after Wio receives and responds to request
                self.send_response(200)
                self.send_header("Content-Type", "text/plain")
                self.end_headers()
                self.wfile.write(b"Beep instruction received")
            case _:
                self._not_found(self.path)


def run_server():
    """ Run the server
    """
    import socket

    server = HTTPServer(("0.0.0.0", s.PORT), RequestHandler)
    hostname = socket.gethostname()
    hostIP = socket.gethostbyname(hostname)
    server_log.info("Starting server at http://%s:%d/ (%s) ...",
                 hostname, s.PORT, hostIP)

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
        server_log.info("Stopping server...")


if __name__ == '__main__':
    run_server()
