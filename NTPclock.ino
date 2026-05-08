#include <stdio.h>
#include "clock.h"

void setup()
{
    Serial.begin(19200);
}

void loop()
{
    clock_update();
}