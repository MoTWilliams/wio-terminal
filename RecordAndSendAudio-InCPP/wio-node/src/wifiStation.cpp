#include <Arduino.h>
#include <rpcWiFi.h>
#include <WiFiClient.h>
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

void WiFiStation::update(unsigned long now) {
        switch (status)
        {
                case Status::OFFLINE: update_offline(now); return;
                case Status::SEARCHING: update_searching(now); return;
                case Status::ONLINE: update_online(now); return;
        }
}

void WiFiStation::update_offline(unsigned long now) {
        // Wait to try again
        if (now - startedWaiting < waitInterval) return;

        // Attempt the connection again
        WiFi.begin(Secrets::ssid(), Secrets::password());
        startedChecking = now;
        lastChecked = now;

        // Try three times, then back off
        if (waitInterval == NORMAL_WAIT_INTERVAL) tries++;

        // Move on to next state
        status = Status::SEARCHING;
}

void WiFiStation::update_searching(unsigned long now) {
        // Stop checking and start waiting after 10 seconds
        if (now - startedChecking >= TIMEOUT)
        {
                status = Status::OFFLINE;
                startedWaiting = now;
                if (waitInterval == NORMAL_WAIT_INTERVAL && tries >= MAX_TRIES)
                {
                        waitInterval = BACKOFF_WAIT_INTERVAL;
                        tries = 0;
                }
                return;
        }

        // Only check after check interval
        if (now - lastChecked < CHECK_INTERVAL) return;
        if (WiFi.status() != WL_CONNECTED) { lastChecked = now; return; }

        // Connection found
        status = Status::ONLINE;
        waitInterval = NORMAL_WAIT_INTERVAL;
        tries = 0;

        IPAddress ip = WiFi.localIP();
        if (Serial) Serial.printf("Connected--IP Address: %u.%u.%u.%u\n", 
                ip[0], ip[1], ip[2], ip[3]);
}

void WiFiStation::update_online(unsigned long now) {
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