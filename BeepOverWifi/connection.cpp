#include <Arduino.h>
#include <rpcWiFi.h>
#include <WiFiClient.h>
#include "arduino_secrets.h"
#include "connection.h"
#include "buzzer.h"

WiFiServer server(80);

void wifi_init() {
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();

        Serial.println("Connecting to WiFi...");
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.println("Still connecting...");
        }

        Serial.println("Connected to the WiFi network");

        Serial.print("IP Address: ");
        Serial.println (WiFi.localIP());
}

void server_init() {
        server.begin();
}

void server_listen() {
        WiFiClient client = server.available();
        if (!client) return;

        Serial.println("Request received");
        buzzer_beep();
        client.stop();
}