
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
        System::now = millis();
        
        System::wifiStation.update();
        System::httpServer.update();
        System::httpClient.update();

        System::dispatcher.update();

        System::recorder.update();
}
