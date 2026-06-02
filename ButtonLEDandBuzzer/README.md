# Sequence Generator — Wio Terminal

A small embedded program for the Seeed Studio Wio Terminal that generates number sequences from button input, with LED, buzzer, and Serial output feedback.

Each button controls a different action. Button A generates the next odd number, Button B generates the next Fibonacci number, and Button C resets both sequences. When a sequence value is generated, the buzzer briefly beeps, the value is printed to the Serial monitor, and the built-in LED blinks the generated number.

The program uses debounced button input to avoid repeated accidental reads from a single press. Input handling, sequence logic, and output behavior are separated into different functions so the program is easier to read, modify, and extend.

The sequence-selection logic uses function pointers so each sequence can store its own recurrence function. This keeps the `get_next()` function reusable without needing a separate switch statement for each sequence type.

## Hardware

* Seeed Studio Wio Terminal
* Built-in top buttons
* Built-in LED
* Built-in buzzer

## Build and Upload

This prototype was built and uploaded using the Arduino IDE.

1. Open the sketch in the Arduino IDE.
2. Select the Wio Terminal board.
3. Connect the Wio Terminal.
4. Upload the sketch to the device.
5. Open the Serial monitor at 115200 baud to view debug output.

Other Wio Terminal development workflows may also work, but Arduino IDE is the workflow used for this prototype.

## Controls

| Button   | Action                             |
| -------- | ---------------------------------- |
| Button A | Generate the next odd number       |
| Button B | Generate the next Fibonacci number |
| Button C | Reset both sequences               |

## Output Behavior

When Button A or Button B is pressed:

1. The buzzer briefly beeps.
2. The next sequence value is printed to the Serial monitor.
3. The LED blinks according to the generated value.

When Button C is pressed:

1. Both sequences reset to their starting values.
2. A reset message is printed to the Serial monitor.

## Notes

This is a small prototype/lab project. It is intended as a learning reference for button input, debouncing, basic output control, and organizing simple embedded logic on the Wio Terminal.

