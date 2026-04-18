#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>  // For boolean type in C

typedef struct Sensor Sensor;
typedef void (*isr_t)(void);
typedef struct Button Button;
struct Button {
  const int pin;
  unsigned long lastPressTime;
  isr_t interrupt;
  Sensor* sensor;
};

void requestAccel();
void requestTemp();

void button_init(Button* b);
bool button_pressed(Button* b);

#endif