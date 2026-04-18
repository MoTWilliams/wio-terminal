/*
  Morgan Williams
  CSCE 3612.001
  Lab 4: Wio Serial and Interrupts
  Apr 22, 2026
*/
#include "button.h"
#include "sensor.h"
#include "buzzer.h"
#include "serial.h"
#include "command.h"

static Sensor idle  = { "idle", NULL, read_Idle };
Sensor temp  = { "AHT20 (temperature)", init_Temp, read_Temp };
Sensor humid = { "AHT20 (humidity)", init_Humid, read_Humid };
Sensor accel = { "accelerometer", init_Accel, read_Accel };
Sensor light = { "light sensor", init_Light, read_Light };

// HIGH = button in released state
Button accel_b = { WIO_KEY_A, 0, requestAccel, &accel };
Button temp_b  = { WIO_KEY_B, 0, requestTemp, &temp };
// WIO_KEY_C is unused in this lab

volatile Button* triggeredButton;
volatile bool sensorRequested;

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
  receiveAndStartSerialCommand();

  // Listen for and execute botton commands
  executeButtonCommand((Button*)triggeredButton);

  // Monitor running actions
  mindSerialCommand();
  mindBuzzer();
}