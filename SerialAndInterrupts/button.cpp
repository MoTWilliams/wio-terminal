#include "WInterrupts.h"
#include <Arduino.h>
#include "button.h"
#include "sensor.h"

extern volatile bool sensorRequested;

void requestSensor() {
  sensorRequested = true;
}

void button_init(Button* b) {
  pinMode(b->pin, INPUT_PULLUP);
  attachInterrupt(b->pin, requestSensor, FALLING);
}

bool button_pressed(Button* b) {
  unsigned long DEBOUNCE_DELAY = 50;
  bool pressed = false;

  // Ignore state changes for 50ms
  if (millis() - b->lastDebounceTime > DEBOUNCE_DELAY)
  {
    
  }

  return pressed;
}