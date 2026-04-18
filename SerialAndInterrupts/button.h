#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>  // For boolean type in C

typedef struct 
typedef void (*isr_t)(void);
typedef struct Button Button;
struct Button {
  const int pin;                  // Physical button
  int lastRead;                   // Previous raw button reading
  int stable;                     // Most recent stable button state
  unsigned long lastPressTime; // Timestamp of last raw state change
  Sensor* sensor;
};

void requestSensor();

void button_init(Button* b);
bool button_pressed(Button* b);

#endif