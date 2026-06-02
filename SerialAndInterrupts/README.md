# Serial and Interrupts — Wio Terminal

A Wio Terminal prototype that reads sensor data using both button interrupts and Serial commands. The program uses interrupt service routines to request sensor readings from button presses, while also accepting timed sensor-read commands through the Serial monitor.

This prototype separates button handling, buzzer control, sensor logic, Serial input, and command execution into separate source files. The goal is to keep the main sketch small while organizing each hardware or control-flow responsibility into its own module.

## Features

* Uses hardware interrupts for button-triggered sensor reads
* Reads commands from the Serial monitor
* Supports timed repeated sensor readings
* Uses debounced button input to avoid accidental repeated presses
* Provides buzzer feedback when a valid button press is registered
* Uses non-blocking update functions for timed commands and buzzer behavior
* Reads from the accelerometer, light sensor, temperature sensor, and humidity sensor
* Uses function pointers to give each sensor its own initialization and read behavior
* Handles unavailable sensors with a fallback “not found” message

## Hardware

* Seeed Studio Wio Terminal
* Built-in top buttons
* Built-in buzzer
* Built-in accelerometer
* Built-in light sensor
* AHT20 temperature/humidity sensor

## Libraries

This prototype uses the following libraries:

* `Arduino`
* `Wire`
* `LIS3DHTR`
* `AHT20`

## File Structure

| File                        | Purpose                                                 |
| --------------------------- | ------------------------------------------------------- |
| `SerialAndInterrupts.ino`   | Main sketch; initializes modules and runs the main loop |
| `button.h` / `button.cpp`   | Button setup, interrupt attachment, and debouncing      |
| `buzzer.h` / `buzzer.cpp`   | Non-blocking buzzer feedback                            |
| `command.h` / `command.cpp` | Button-command and Serial-command execution             |
| `sensor.h` / `sensor.cpp`   | Sensor initialization and sensor-reading functions      |
| `serial.h` / `serial.cpp`   | Serial setup and non-blocking line reading              |

## Build and Upload

This prototype was built and uploaded using the Arduino IDE.

1. Open `SerialAndInterrupts.ino` in the Arduino IDE.
2. Make sure the related `.h` and `.cpp` files are in the same sketch folder.
3. Select the Wio Terminal board.
4. Connect the Wio Terminal.
5. Upload the sketch to the device.
6. Open the Serial monitor at 19200 baud.

Other Wio Terminal development workflows may also work, but Arduino IDE is the workflow used for this prototype.

## Button Controls

| Button   | Action                           |
| -------- | -------------------------------- |
| Button A | Request an accelerometer reading |
| Button B | Request a temperature reading    |

Button presses are handled through interrupts. The interrupt service routine only records that a button request occurred. The actual sensor read is handled later in the main loop.

If a timed Serial command is already running, button-triggered reads are blocked and the program prints:

```text
Timed operation in progress
```

## Serial Commands

Serial commands use the following format:

```text
READ <SENSOR> <REPEATS>, <INTERVAL>
```

Supported sensor codes:

| Sensor Code | Sensor          |
| ----------- | --------------- |
| `LGHT`      | Light sensor    |
| `HUMD`      | Humidity sensor |

`REPEATS` must be a single digit from 1 to 5.

`INTERVAL` must be a single digit from 1 to 5, measured in seconds.

Examples:

```text
READ LGHT 3, 1
```

Reads the light sensor 3 times, once every 1 second.

```text
READ HUMD 5, 2
```

Reads the humidity sensor 5 times, once every 2 seconds.

If the command is invalid, the program prints:

```text
Invalid Command
```

When a valid command finishes, the program prints:

```text
Executed
```

## Output Behavior

Sensor readings are printed to the Serial monitor.

Supported outputs include:

* Temperature in Celsius
* Humidity as a percentage
* Accelerometer X, Y, and Z values
* Light sensor analog value

The buzzer briefly beeps when a valid button press is accepted.

## Program Design

The main sketch stays small by delegating most behavior to module-specific functions.

The main loop repeatedly:

1. Checks for new Serial commands.
2. Executes pending button interrupt requests.
3. Continues any active timed Serial command.
4. Updates button debounce state.
5. Updates buzzer state.

The timed Serial command and buzzer behavior are non-blocking. Instead of stopping the program with `delay()`, the code checks elapsed time with `millis()` and continues work only when the correct amount of time has passed.

## Notes

This is a prototype/lab project. It is intended as a learning reference for interrupts, Serial input parsing, button debouncing, function pointers, non-blocking timing, and modular embedded program organization on the Wio Terminal.
