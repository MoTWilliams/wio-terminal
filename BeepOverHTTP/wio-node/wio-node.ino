#include "serial.h"
#include "comm.h"
#include "buzzer.h"

void setup() {
        serial_init();

        wifiStation_init();
        server_init();

        buzzer_init();
}

void loop() {
        server_update();
        buzzer_update();
}
