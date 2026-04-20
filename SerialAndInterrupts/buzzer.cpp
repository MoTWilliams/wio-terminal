/*
  Morgan Williams
  CSCE 3612.001
  Lab 4: Wio Serial and Interrupts - buzzer.cpp
  Apr 22, 2026
*/
#include <Arduino.h>
#include "buzzer.h"

#define BUZZER WIO_BUZZER
#define ON 128
#define OFF 0

// Tracks if beep is currently active
static bool isBuzzing = false;

// Duration of beep in miliseconds
static unsigned long BEEP_DURATION = 250;

// Timestamp when the current beep started
static unsigned long lastStarted = 0;

void buzzer_init() {
  pinMode(BUZZER, OUTPUT);
  analogWrite(BUZZER, OFF);
}

void buzzer_beep() {
  // Start a non-blocking beep
  analogWrite(BUZZER, ON);

  isBuzzing = true;
  lastStarted = millis();
}

void buzzer_update() {
  if (!isBuzzing) return;

  // Continue sounding buzzer until duration has elapsed
  if (millis() - lastStarted < BEEP_DURATION) return;

  analogWrite(BUZZER, OFF);
  isBuzzing = false;
}

bool buzzer_isBuzzing() { return isBuzzing; }