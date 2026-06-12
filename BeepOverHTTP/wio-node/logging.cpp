#include <Arduino.h>
#include "logging.h"

typedef enum {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        CRITICAL
} Levels;

static void send_log(int level, char* msg) {
        int MAX_LEN = 512;
        char buffer[MAX_LEN + 1] = {0};
}

