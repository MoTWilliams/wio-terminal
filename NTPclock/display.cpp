#include <stdio.h>
#include "display.h"

extern uint8_t h, m, s;

void display_buildClockDisplay(char* buffer, size_t size) {
  snprintf(buffer, size, "%02d:%02d:%02d\n", h, m, s);
}