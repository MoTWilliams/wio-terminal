#include <Arduino.h>
#include <rpcWiFi.h>
#include <WiFiClient.h>
#include <stdbool.h>
#include "network.h"
#include "arduino_secrets.h"

typedef enum {
        OFFLINE,
        SEARCHING,
        CONNECTED
} WiFiStatus;

static WiFiStatus status = OFFLINE;
static bool offlineMode = true;

void wifiStation_init(void) {
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();      // Drop existing connections
}

bool wifiStation_isOffline(void) {
        return offlineMode;
}

void wifiStation_onDisconnected(void) {
        // Prevent repeated resets
        if (status == OFFLINE) return;
        
        status = OFFLINE;
        offlineMode = true;
}

bool wifiStation_checkClientConnection(
                WiFiClient client, unsigned long *lastProgress) {
        static const unsigned long TIMEOUT = 3000;
        
        if (!client.connected())
        {
                wifiStation_handleClientConnectionLost(client);
                return false;
        }

        if (lastProgress && millis() - *lastProgress >= TIMEOUT)
        {
                wifiStation_handleClientConnectionLost(client);
                return false;
        }
        return true;
}

/* Call reset*() before calling this */
void wifiStation_handleClientConnectionLost(WiFiClient client) {
        if (WiFi.status() != WL_CONNECTED)
                wifiStation_onDisconnected();
}

void wifiStation_update(void) {
        static const unsigned long TIMEOUT_INTERVAL = 10000;
        static const unsigned long CHECK_INTERVAL = 1000;
        static const unsigned long NORMAL_WAIT_INTERVAL = 5000;
        static const unsigned long BACKOFF_WAIT_INTERVAL = 60000;
        static const int MAX_TRIES = 3;

        unsigned long now = millis();
        static unsigned long startedChecking = now;
        static unsigned long lastChecked = now;
        static unsigned long waitInterval = NORMAL_WAIT_INTERVAL;
        static unsigned long startedWaiting = now;
        static int tries = 0;

        if (status == OFFLINE)
        {
                if (now - startedWaiting >= waitInterval)
                {
                        // Attempt a new connection
                        WiFi.begin(SSID, PASSWORD);
                        startedChecking = now;
                        lastChecked = now;
                        if (waitInterval == NORMAL_WAIT_INTERVAL) tries++;
                        
                        status = SEARCHING;
                        return;
                }
        }

        if (status == SEARCHING)
        {
                // Stop checking after 10 seconds
                if (now - startedChecking >= TIMEOUT_INTERVAL)
                {
                        status = OFFLINE;
                        offlineMode = true;
                        startedWaiting = now;
                        if (waitInterval == NORMAL_WAIT_INTERVAL 
                                && tries >= MAX_TRIES)
                        {
                                waitInterval = BACKOFF_WAIT_INTERVAL;
                                tries = 0;
                        }

                        return;
                }
                
                // Only check after check interval
                if (now - lastChecked < CHECK_INTERVAL) return;
                if (WiFi.status() != WL_CONNECTED) 
                {
                        lastChecked = now;
                        return;
                }

                // Connection found
                status = CONNECTED;
                Serial.println("Wifi connected");
                offlineMode = false;
                waitInterval = NORMAL_WAIT_INTERVAL;
                tries = 0;
                return;
        }

        if (status == CONNECTED)
        {
                if (WiFi.status() != WL_CONNECTED)
                {
                        wifiStation_onDisconnected();
                        return;
                }
        }
}
