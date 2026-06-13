// https://wiki.seeedstudio.com/Wio-Terminal-Wi-Fi/#configure-as-ap-mode-simple-web-server-example-code
// https://wiki.seeedstudio.com/Wio-Terminal-Wi-Fi/#wi-fi-client-example-code
#include <Arduino.h>
#include <string.h>
#include <stdbool.h>
#include <rpcWiFi.h>
#include <WiFiClient.h>
#include "comm.h"
#include "logging.h"
#include "arduino_secrets.h"
#include "buzzer.h"


typedef enum {
        WAITING,
        RECEIVING_HEADERS,
        RECEIVING_BODY,
        RESPONDING,
        SENDING
} RequestStatuses;

int requestStatus = WAITING;
WiFiClient client;

/******************************************************************************
 *                                  Server                                    *
 ******************************************************************************/

WiFiServer server(8080);

void server_init(void) {
        server.begin();
}

void server_update(void) {
        if (requestStatus == WAITING)
        {
                client = server.available();
                if (!client) return;
                Serial.println("Got client");
                requestStatus = RECEIVING_HEADERS;
                return;
        }

        static int bodyLen;
        static int readBytes = 0;
        static char lineBuffer[512 + 1] = {0};

        if (requestStatus == RECEIVING_HEADERS)
        {
                if (!client.connected())
                {
                        client.stop();
                        requestStatus = WAITING;
                        return;
                }

                if (!client.available()) return;

                // Parse headers
                static char c, *p = lineBuffer;

                c = client.read();

                if (c == '\n')
                {
                        if (strlen(lineBuffer) == 0)
                        {
                                Serial.println("Blank line detected. End of header");
                                Serial.println();
                                requestStatus = RECEIVING_BODY;
                                return;
                        }
                        else
                        {
                                Serial.println(lineBuffer);

                                // Get body size (bytes)
                                char *goal = "Content-Length: ";
                                int len = 16;

                                if (strncmp(lineBuffer, goal, len) == 0)
                                {
                                        char bodyLenStr[32 + 1] = {0};
                                        char *dgt = lineBuffer + len;
                                        char *d = bodyLenStr;

                                        while (*dgt != '\0')
                                        {
                                                *d = *dgt;
                                                dgt++;
                                                d++;
                                        }

                                        bodyLen = atoi(bodyLenStr);
                                        readBytes = 0;
                                        Serial.printf("# body bytes = %d\n", bodyLen);
                                }

                                p = lineBuffer;
                                *p = '\0';
                        }
                }
                else if (c != '\r')
                {
                        *p = c;
                        *(p + 1) = '\0';
                        p++;
                }
        }

        if (requestStatus == RECEIVING_BODY)
        {
                
                
                static char *p;

                if (readBytes == 0)
                {
                        p = lineBuffer;
                        *p = '\0';
                }

                if (readBytes < bodyLen) 
                {
                        if (!client.available()) return;

                        *p = client.read();
                        Serial.write(*p);
                        p++;
                        *p = '\0';
                        readBytes++;
                }

                if (readBytes == bodyLen)
                {
                        requestStatus = RESPONDING;
                        return;
                }
        }

        if (requestStatus == RESPONDING)
        {
                Serial.printf("\nbody = \"%s\"\n\n", lineBuffer);
                if (strcmp(lineBuffer, "beep") == 0) buzzer_beep();
                
                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: test/plain");
                client.println("Connection: close");
                client.println();
                client.print("Wio: Beep request handled");
                client.println();

                client.stop();
                requestStatus = WAITING;
                return;
        }
}

/******************************************************************************
 *                                  Client                                    *
 ******************************************************************************/

/******************************************************************************
 *                                 Station                                    *
 ******************************************************************************/

void wifiStation_init(void) {
        unsigned long TIMEOUT = 10000;
        unsigned long CHECK_INTERVAL = 1000;
        
        WiFi.mode(WIFI_STA);    // Configure as station
        WiFi.disconnect();      // Drop any existing connections
        
        WiFi.begin(SSID, PASSWORD);
        unsigned long now = millis();
        unsigned long connectionStarted = now;
        unsigned long lastChecked = now;
        bool connectionFound = false;

        Serial.println("Connecting to Wi-Fi...");
        while (!connectionFound && now - connectionStarted < TIMEOUT)
        {
                // Make the loop less busy. This delay can be removed if this
                // logic is moved into loop() to run repeatedly whenever
                // connection is lost
                delay(100);
                now = millis();

                // Only check after CHECK_INTERVAL has passed
                if (now - lastChecked < CHECK_INTERVAL) continue;
                
                // Keep checking if no connection found
                Serial.println("Checking connection...");
                if (WiFi.status() != WL_CONNECTED) lastChecked = now;
                else connectionFound = true;
        }

        // Halt if no connection established after TIMEOUT
        if (!connectionFound) 
        {
                Serial.print("Connection failed: ");
                Serial.println(WiFi.status());
                while (true);
        }

        // Otherwise, announce address and begin loop()
        IPAddress ip = WiFi.localIP();

        char ipBuffer[16];
        snprintf(
                ipBuffer, sizeof(ipBuffer), 
                "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]
        );

        Serial.printf("Connected--IP Address: %s\n", ipBuffer);
}
