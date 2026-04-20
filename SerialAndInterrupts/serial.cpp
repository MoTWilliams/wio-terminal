/*
  Morgan Williams
  CSCE 3612.001
  Lab 4: Wio Serial and Interrupts - serial.cpp
  Apr 22, 2026
*/
#include <stddef.h>
#include <Arduino.h>
#include "serial.h"

void serial_init() {
  Serial.begin(19200);
}

void serial_readline(char* buffer, size_t size) {
  if (size <= 0) return;
  unsigned int i = 0;
  char c = 0;

  // Non-blocking read: copy available serial data into buffer until newline or
  // the buffer is full
  while (Serial.available())
  {
    // Don't run off the end of the buffer--truncate and null-terminate
    if (i == size-1) { buffer[i] = '\0'; break; }

    c = Serial.read();

    // Discard carriage returns. Stop reading on newline
    if (c == '\r') continue;
    if (c == '\n') { buffer[i] = '\0'; break; }

    buffer[i++] = c;
  }

  // Null-terminate if no newline found
  buffer[i] = '\0';
}