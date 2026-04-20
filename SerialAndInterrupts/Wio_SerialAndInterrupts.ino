/*
  Morgan Williams
  CSCE 3612.001
  Lab 4: Wio Serial and Interrupts - Wio_SerialAndInterrupts.ino
  Apr 22, 2026
*/
#include "button.h"
#include "sensor.h"
#include "buzzer.h"
#include "serial.h"
#include "command.h"

Sensor temp  = { "AHT20 (temperature)", init_Temp, read_Temp };
Sensor humid = { "AHT20 (humidity)", init_Humid, read_Humid };
Sensor accel = { "accelerometer", init_Accel, read_Accel };
Sensor light = { "light sensor", init_Light, read_Light };

Button accel_b = { WIO_KEY_A, 0, requestAccel, &accel, false };
Button temp_b  = { WIO_KEY_B, 0, requestTemp, &temp, false };

// Identifies which button triggered an interrupt request. SET BY button ISRs.
// SHARED WITH command module.
volatile Button* triggeredButton;

void setup() {
  serial_init();

  buzzer_init();

  button_init(&accel_b);
  button_init(&temp_b);

  sensor_init(&temp);
  sensor_init(&humid);
  sensor_init(&accel);
  sensor_init(&light);
}

void loop() {
  // Listen for serial commands. If one is received, begin executing it.
  serialCommand_execute();

  // Execute commands from button interrupts
  buttonCommand_execute((Button*)triggeredButton);

  // Monitor running actions
  serialCommand_update();
  button_update((Button*)triggeredButton);
  buzzer_update();
}