#include "serial.h"
#include "comm.h"
#include "buzzer.h"
#include "button.h"

void setup() {
        serial_init();

        wifiStation_init();
        server_init();

        button_init();
        buzzer_init();
}

void loop() {
        static unsigned long now;
        
        server_update();

        now = millis();
        button_update(now);
        buzzer_update(now);
}
