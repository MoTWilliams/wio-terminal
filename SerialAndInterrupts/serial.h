/*
  Morgan Williams
  CSCE 3612.001
  Lab 4: Wio Serial and Interrupts - serial.h
  Apr 22, 2026
*/
#ifndef SERIAL_H
#define SERIAL_H

#include <stddef.h>  // for size_t

void serial_init();

// Read line from serial input into buffer (non-blocking)
void serial_readline(char* buffer, size_t size); 

#endif