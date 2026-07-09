# pylint: disable=invalid-name
# pylint: disable=global-statement
""" Prototype to record audio from Wio Terminal and send to Python server """
import tkinter as tk
from requests import post, RequestException
import paths as p
# import server

window = None
button = None
finished_recording = False

def start_recording():
    try:
        response = post(f"{p.WIO_URL}record/start", timeout=2)
        print("Wio response:", response.text)
        button.config(text="Recording...")
    except RequestException as error:
        print(f"Could not start recording: {error}")

def mark_recording_finished():
    global finished_recording
    finished_recording = True

def check_server_events():
    global finished_recording
    if finished_recording:
        finished_recording = False
        button.config(text="Start recording")

    window.after(100, check_server_events)

def reset_button():
    button.config(text="Start recording")

def run_GUI():
    global window, button

    window = tk.Tk()
    window.title("Recorder")

    button = tk.Button(
        window, text="Start recording", command=start_recording)
    button.pack(padx=20, pady=10)

    window.bind("<Escape>", lambda event: window.destroy())
    check_server_events()
    window.mainloop()
