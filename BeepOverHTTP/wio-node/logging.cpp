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

static bool validate(const char *fmt) {
        for (const char *p = fmt; *p != '\0'; p++)
        {
                // See '%' and look at the next character
                if (*p != '%') continue;
                p++;

                // Don't allow bare '%' at the end
                if (*p == '\0') return false;

                // Literal '%', or char, string, int, or unsigned int
                if (strchr("csdu%", *p)) continue;

                // See 'l' and look at the next character
                if (*p != 'l') return false;
                p++;

                // Don't allow bare '%l' at end
                if (*p == '\0') return false;

                // Only signed and unsigned ints can be long
                if (*p == 'd' || *p == 'u') continue;
                
                // Anything else is invalid
                return false;
        }

        // If we get all the way to the end, it's a valid string
        return true;
}

static void buildMsg(char* buffer, size_t size, const char *fmt, ...) {
        if (!validate(fmt))
        {
                Serial.println("Malformed log message: Invalid string");
                return;
        }
        
        // String is valid. Perform substitutions        
        va_list args;
        va_start(args, fmt);
        int status = vsnprintf(buffer, size, fmt, args);
        va_end(args);

        if (status < 0 || status >= MAX_LEN)
                Serial.println("Malformed log message: Too long?");
        else
                Serial.println(buffer);
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

