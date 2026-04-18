#include "WInterrupts.h"
#include <Arduino.h>
#include "button.h"
#include "buzzer.h"
#include "sensor.h"

extern Button accel_b;
extern Button temp_b;

extern volatile Button* triggeredButton;
extern volatile bool sensorRequested;

void requestAccel() {
  sensorRequested = true;
  triggeredButton = &accel_b;
}

void requestTemp() {
  sensorRequested = true;
  triggeredButton = &temp_b;
}

void button_init(Button* b) {
  pinMode(b->pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(b->pin), b->interrupt, FALLING);
}

bool button_pressed(Button* b) {
  if (!sensorRequested) return false;
  if (triggeredButton != b) return false;

  sensorRequested = false;
  
  unsigned long DEBOUNCE_DELAY = 50;
  unsigned long now = millis();

  // Ignore state changes for 50ms
  if (now - b->lastPressTime > DEBOUNCE_DELAY)
  {
    b->lastPressTime = now;
    beep();
    return true;
  }

  return false;
}