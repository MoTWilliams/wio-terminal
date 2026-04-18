#include <stddef.h>
#include <Arduino.h>
#include "serial.h"

void serial_init() {
  Serial.begin(19200);
  while (!Serial) { /* Spin until ready */ }
}

// https://docs.arduino.cc/language-reference/en/functions/communication/serial/available/
// https://docs.arduino.cc/language-reference/en/functions/communication/serial/read/
void serial_readline(char* buffer, size_t size) {
  unsigned int i = 0;
  char c = 0;
  while (Serial.available())
  {
    // Don't run off the end of the buffer--truncate
    if (i == size) break;

    c = Serial.read();

    // Discard carriage returns. Stop reading on newline
    if (c == '\r') continue;
    if (c == '\n') break;

    buffer[i++] = c;
  }
}