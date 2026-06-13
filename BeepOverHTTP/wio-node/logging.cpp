#include <Arduino.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include "logging.h"

static bool serialActive = false;

typedef enum {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        CRITICAL
} Levels;

void build_log(const char *msg, ...) {
        

        char typesBuffer[256 + 1] = {0};
        char *t_p = typesBuffer;
        bool validString = true;
        for (const char *p = msg; *p != '\0'; p++)
        {
                // Abort parsing if the last character invalidated the string
                if (!validString) break;
                
                // Just add the character to the buffer (eventually)
                if (*p != '%') { p++; continue; }
                
                switch (*(p + 1))
                {
                        case 'l':
                                if (*(p + 2) == 'u') 
                                {
                                        *t_p++ = 'U';
                                        p++;
                                        break;
                                }
                                
                                // Error: incomplete format specifier
                                validString = false;
                                break;
                        case '%':
                                *t_p++ = '%';
                                break;
                        case 's':
                                *t_p++ = 's';
                                break;
                        case 'd':
                                *t_p++ = 'd';
                                break;
                        case 'u':
                                *t_p++ = 'u';
                                break;
                        default:
                                validString = false;
                                break;
                }
        }

        if (!validString)
                Serial.println("Malformed log string");
        else
                Serial.printf("Types: %s\n", typesBuffer);
}




void log_init(void) {
        // Attempt to initiate serial connection
        Serial.begin(115200);

        // If no connection found, continue without serial output
        unsigned long startedAt = millis();
        while (!serialActive && millis() - startedAt < 500) // 1/2 second
        {
                delay(100);
                if (Serial) serialActive = true;
        }
}

