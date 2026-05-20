#include <Arduino.h>
#include "clock.h"
#include "display.h"

uint8_t h = 0, m = 0, s = 0;

void clock_update() {
  static const unsigned long SECOND = 1000;
  static unsigned long now, lastSec = 0;

  now = millis();

  if (now - lastSec < SECOND) return;
  lastSec = now;

  s++;
  if (s >= 60) { m++; s = 0; }
  if (m >= 60) { h++; m = 0; }
  if (h >= 24) h = 0;

  char buffer[64] = {0};
  display_buildClockDisplay(buffer, sizeof(buffer));
  Serial.print(buffer);
}

