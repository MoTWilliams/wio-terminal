#ifndef BUZZER_H
#define BUZZER_H

void buzzer_init(void);
void buzzer_beep(void);         // Start a non-blocking beep
void buzzer_update(unsigned long now);       // Call in main loop to maintain buzzer state

#endif