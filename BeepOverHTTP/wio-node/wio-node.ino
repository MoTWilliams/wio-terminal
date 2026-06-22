#include "logging.h"
#include "comm.h"
#include "buzzer.h"
#include "button.h"

void setup() {
        log_init();

        wifiStation_init();
        server_init();

        button_init();
        buzzer_init();
}

void loop() {
        server_update();
        client_update();
        button_update();
        buzzer_update();
}
