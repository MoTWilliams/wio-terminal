#include <Arduino.h>
#include <rpcWiFi.h>
#include <WiFiClient.h>
#include "systemObjects.h"
#include "secrets.h"
#include "wifiStation.h"


namespace {
        constexpr unsigned long TIMEOUT = 10000;
        constexpr unsigned long CHECK_INTERVAL = 1000;
        constexpr unsigned long NORMAL_WAIT_INTERVAL = 5000;
        constexpr unsigned long BACKOFF_WAIT_INTERVAL = 60000;
        constexpr int MAX_TRIES = 3;
}

WiFiStation::WiFiStation() : waitInterval(NORMAL_WAIT_INTERVAL) {}

void WiFiStation::begin() {
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
}

/******************************************************************************/

void WiFiStation::update() {
        switch (status)
        {
                case Status::OFFLINE: update_offline(); return;
                case Status::SEARCHING: update_searching(); return;
                case Status::ONLINE: update_online(); return;
        }
}

void WiFiStation::update_offline() {
        // Wait to try again
        if (System::now - startedWaiting < waitInterval) return;

        // Attempt the connection again
        WiFi.begin(Secrets::ssid(), Secrets::password());
        startedChecking = System::now;
        lastChecked = System::now;

        // Try three times, then back off
        if (waitInterval == NORMAL_WAIT_INTERVAL) tries++;

        // Move on to next state
        status = Status::SEARCHING;
}

void WiFiStation::update_searching() {
        // Stop checking and start waiting after 10 seconds
        if (System::now - startedChecking >= TIMEOUT)
        {
                status = Status::OFFLINE;
                startedWaiting = System::now;
                if (waitInterval == NORMAL_WAIT_INTERVAL && tries >= MAX_TRIES)
                {
                        waitInterval = BACKOFF_WAIT_INTERVAL;
                        tries = 0;
                }
                return;
        }

        // Only check after check interval
        if (System::now - lastChecked < CHECK_INTERVAL) return;
        if (WiFi.status() != WL_CONNECTED) { lastChecked = System::now;return; }

        // Connection found
        status = Status::ONLINE;
        waitInterval = NORMAL_WAIT_INTERVAL;
        tries = 0;

        IPAddress ip = WiFi.localIP();
        if (Serial) Serial.printf("Connected--IP Address: %u.%u.%u.%u\n", 
                ip[0], ip[1], ip[2], ip[3]);
}

void WiFiStation::update_online() {
        if (WiFi.status() != WL_CONNECTED) onDisconnected();
}

/******************************************************************************/

void WiFiStation::onDisconnected() {
        if (status == Status::OFFLINE) return;
        status = Status::OFFLINE;
}

bool WiFiStation::isOffline() {
        if (status == Status::ONLINE) return false;
        return true;
}