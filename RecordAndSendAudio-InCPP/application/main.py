""" Run the Python application """
import threading
import server
import gui



if __name__ == '__main__':
    # Run HTTP server in a background thread
    server_thread = threading.Thread(target=server.run_server, daemon=True)
    server_thread.start()

    gui.run_GUI()
