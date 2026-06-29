#include "network.h"
#include "recorder.h"
#include <stdbool.h>

void setup() {
        Serial.begin(115200);

        wifiStation_init();
        server_init();
}

void loop() {
        wifiStation_update();
        server_update();
        client_update();

        recorder_update();
}
