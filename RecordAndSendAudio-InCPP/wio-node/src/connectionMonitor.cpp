#include <Arduino.h>
#include <rpcWiFi.h>
#include <WiFiClient.h>
#include "systemObjects.h"
#include "connectionMonitor.h"

namespace {
        constexpr unsigned long TIMEOUT = 3000;
}

ConnectionMonitor::ConnectionMonitor() {}

bool ConnectionMonitor::checkConnection(WiFiClient &client) {
        if (!client.connected()) { handleConnectionLost(); return false; }
        return true;
}

bool ConnectionMonitor::checkConnection(
                WiFiClient &client, unsigned long lastProgress) {
        if (!client.connected() || millis() - lastProgress >= TIMEOUT)
        {
                handleConnectionLost();
                return false;
        }
        return true;
}

void ConnectionMonitor::handleConnectionLost() {
        if (WiFi.status() != WL_CONNECTED) System::wifiStation.onDisconnected();
}