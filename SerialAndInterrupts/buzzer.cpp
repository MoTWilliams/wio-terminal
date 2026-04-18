#include <Arduino.h>
#include "buzzer.h"

#define BUZZER WIO_BUZZER
#define ON 128
#define OFF 0

static bool isBuzzing = false;
unsigned long BEEP_DURATION = 250;
unsigned long lastStarted = 0;

void buzzer_init() {
  pinMode(BUZZER, OUTPUT);
  analogWrite(BUZZER, 0);
}

void beep() {
  analogWrite(BUZZER, ON);

  isBuzzing = true;
  lastStarted = millis();
}

void mindBuzzer() {
  if (!isBuzzing) return;

  // Continue sounding buzzer if the beep duration has not yet passed
  if (millis() - lastStarted < BEEP_DURATION) return;

  analogWrite(BUZZER, OFF);
  isBuzzing = false;
}

bool buzzer_isBuzzing() { return isBuzzing; }