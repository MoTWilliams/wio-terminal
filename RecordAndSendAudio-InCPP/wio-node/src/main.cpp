
#include <Arduino.h>
#include "systemObjects.h"

void setup() {
        Serial.begin(115200);
        delay(10000);   // Necessary to see initialization in Serial log

        Serial.println("Starting...");

        System::sdCard.begin();
        System::wifiStation.begin();
        System::httpServer.begin();
}

void loop() {
        unsigned long now = millis();
        
        System::wifiStation.update(now);
        System::httpServer.update(now);
        System::httpClient.update(now);

        System::dispatcher.update(now);

        System::recorder.update(now);
}
