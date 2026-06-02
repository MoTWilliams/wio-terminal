# LCD and Sensors — Wio Terminal

A Wio Terminal prototype that reads sensor data and displays it on the built-in LCD screen. The program uses the top buttons to cycle between different sensor modes, then prints the current reading to both the LCD and the Serial monitor.

The prototype supports temperature/humidity, accelerometer, and light sensor readings. It also includes a basic idle state, debounced button input, buzzer feedback, and a small linked-state-machine structure for switching between sensor modes.

## Features

* Displays sensor readings on the Wio Terminal LCD
* Prints sensor readings to the Serial monitor
* Uses Button A and Button B to cycle through sensor modes
* Uses Button C to reset the display and return to idle
* Uses buzzer feedback when buttons are pressed
* Debounces button input to avoid repeated accidental presses
* Handles missing temperature/humidity or accelerometer hardware by displaying a “not found” message
* Uses a simple linked state machine to move between sensor modes

## Hardware

* Seeed Studio Wio Terminal
* Built-in LCD screen
* Built-in top buttons
* Built-in buzzer
* Built-in light sensor
* Built-in accelerometer
* AHT20 temperature/humidity sensor

## Libraries

This prototype uses the following libraries:

* `Wire`
* `LIS3DHTR`
* `TFT_eSPI`
* `AHT20`

## Build and Upload

This prototype was built and uploaded using the Arduino IDE.

1. Open the sketch in the Arduino IDE.
2. Select the Wio Terminal board.
3. Connect the Wio Terminal.
4. Upload the sketch to the device.
5. Open the Serial monitor at 115200 baud to view debug output.

Other Wio Terminal development workflows may also work, but Arduino IDE is the workflow used for this prototype.

## Controls

| Button   | Action                              |
| -------- | ----------------------------------- |
| Button A | Cycle forward through sensor modes  |
| Button B | Cycle backward through sensor modes |
| Button C | Clear the LCD and return to idle    |

## Sensor Modes

| Mode                 | Output                                                |
| -------------------- | ----------------------------------------------------- |
| Idle                 | Displays `idle`                                       |
| Temperature/Humidity | Displays temperature in Celsius and relative humidity |
| Accelerometer        | Displays X, Y, and Z acceleration values              |
| Light Sensor         | Displays the analog light sensor value                |

## Output Behavior

The LCD updates every 100 ms. Each reading is also printed to the Serial monitor.

The temperature/humidity sensor is read every 2 seconds because the sensor read is slower and blocking. The most recent temperature/humidity reading is cached and reused between reads.

When the LCD fills with text, the screen is cleared and output starts again from the top.

## Notes

This is a prototype/lab project. It is intended as a learning reference for LCD output, sensor input, button debouncing, buzzer feedback, and basic state-machine organization on the Wio Terminal.
