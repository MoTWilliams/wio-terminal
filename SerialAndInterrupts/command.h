/*
  Morgan Williams
  CSCE 3612.001
  Lab 4: Wio Serial and Interrupts - command.h
  Apr 22, 2026
*/
#ifndef COMMAND_H
#define COMMAND_H

typedef struct Button Button;

void requestAccel();                    // Accelerometer button-press ISR
void requestTemp();                     // Temperature button-press ISR

void buttonCommand_execute(Button* b);  // Execute button-triggered sensor req.
void serialCommand_execute();           // Validate and start serial command
void serialCommand_update();            // Continues timed serial command

#endif