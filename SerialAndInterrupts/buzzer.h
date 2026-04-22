/*
  Morgan Williams
  CSCE 3612.001
  Lab 4: Wio Serial and Interrupts - buzzer.h
  Apr 22, 2026
*/
#ifndef BUZZER_H
#define BUZZER_H

void buzzer_init();
void buzzer_beep();         // Start a non-blocking beep
void buzzer_update();       // Call in main loop to maintain buzzer state

#endif