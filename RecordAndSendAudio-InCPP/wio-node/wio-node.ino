#include "wifiStation.h"
#include "httpServer.h"
#include "httpClient.h"
#include "connectionMonitor.h"
#include "recorder.h"

WiFiStation wifiStation;
ConnectionMonitor connectionMonitor(wifiStation);
HTTPClient httpClient(wifiStation, connectionMonitor);
Recorder recorder(httpClient);
HTTPServer httpServer(wifiStation, connectionMonitor, recorder);

void setup() {
        Serial.begin(115200);

        wifiStation.begin();
        httpServer.begin();

        recorder.begin();
}

void loop() {
        unsigned long now = millis();
        
        wifiStation.update(now);
        httpServer.update(now);
        httpClient.update(now);

        recorder.update(now);
}
