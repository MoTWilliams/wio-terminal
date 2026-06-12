#ifndef BUZZER_H
#define BUZZER_H

void buzzer_init();
void buzzer_beep();         // Start a non-blocking beep
void buzzer_update();       // Call in main loop to maintain buzzer state

#endif