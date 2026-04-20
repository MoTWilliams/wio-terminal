/*
  Morgan Williams
  CSCE 3612.001
  Lab 4: Wio Serial and Interrupts - button.cpp
  Apr 22, 2026
*/
#include <Arduino.h>
#include "button.h"
#include "buzzer.h"
#include "sensor.h"

// Minimum time in ms between button presses
static const unsigned long DEBOUNCE_DELAY = 100;

// Cached timestamp for debounce timing
static unsigned long now = 0;

void button_init(Button* b) {
  // INPUT_PULLUP = pin HIGH until pressed. FALLING = trigger interrupt on press
  pinMode(b->pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(b->pin), b->interrupt, FALLING);
}

bool button_pressed(Button* b) {
  // Ignore while waiting for button release
  if (b->waitingForRelease) return false;
  
  now = millis();

  // Ignore presses within the debounce window
  if (now - b->lastPressTime < DEBOUNCE_DELAY) return false;

  // Register valid press
  b->lastPressTime = now;
  b->waitingForRelease = true;
  
  // Audible user feedback
  buzzer_beep();
  
  return true;
}

void button_update(Button* b) {
  if (!b) return;
  
  // Only check for release after registering a valid press
  if (!b->waitingForRelease) return;
  
  now = millis();

  // Only allow the next button press after the current is released and the
  // debounce delay has passed
  if(digitalRead(b->pin) == HIGH && now - b->lastPressTime >= DEBOUNCE_DELAY) 
    b->waitingForRelease = false;
}