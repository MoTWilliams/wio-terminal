#include <Arduino.h>
#include "serial.h"

static unsigned long WAIT_DURATION = 500; // 1/2 second

void serial_init() {
        Serial.begin(115200);
        unsigned long startedAt = millis();
        while (!Serial)
        {
                // Continue without serial input (headless) if a connection is
                // not found in half a second 
                if (millis() - startedAt >= WAIT_DURATION) break;
        }
}