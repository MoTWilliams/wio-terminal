#include <Arduino.h>
#include "buzzer.h"

void buzzer_init() {
        pinMode(WIO_BUZZER, OUTPUT);
        analogWrite(WIO_BUZZER, 0);
}

void buzzer_beep() {
        analogWrite(WIO_BUZZER, 128);
        delay(250);
        analogWrite(WIO_BUZZER, 0);
}