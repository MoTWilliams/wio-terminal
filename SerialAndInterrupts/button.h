/*
  Morgan Williams
  CSCE 3612.001
  Lab 4: Wio Serial and Interrupts - button.h
  Apr 22, 2026
*/
#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

typedef struct Sensor Sensor;
typedef void (*isr_t)(void);
typedef struct Button Button;
struct Button {
  const int pin;
  unsigned long lastPressTime;
  isr_t interrupt;                  // ISR called on button press
  Sensor* sensor;                   // Sensor associated with this button
  bool waitingForRelease;           // Used for debouncing
};

void button_init(Button* b);
bool button_pressed(Button* b);     // Returns true on valid debounced press
void button_update(Button* b);      // Handles release debouncing

#endif