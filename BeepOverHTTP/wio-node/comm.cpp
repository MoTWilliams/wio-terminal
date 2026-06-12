// https://wiki.seeedstudio.com/Wio-Terminal-Wi-Fi/#configure-as-ap-mode-simple-web-server-example-code
// https://wiki.seeedstudio.com/Wio-Terminal-Wi-Fi/#wi-fi-client-example-code
#include <Arduino.h>
#include <string.h>
#include <rpcWiFi.h>
#include <WiFiClient.h>
#include "comm.h"
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

void wifiStation_init() {
        WiFi.mode(WIFI_STA);    // Configure as station
        WiFi.disconnect();      // Drop any existing connections
        WiFi.begin(SSID, PASSWORD);

        while (WiFi.status() != WL_CONNECTED) delay(500);

        WIO_IP = WiFi.localIP().toString().c_str();
        Serial.printf("Connected--IP Address: %s\n", WIO_IP);
}

WiFiServer server(8080);
WiFiClient client;

void server_init() {
        server.begin();
}

void server_update() {
        if (requestStatus == WAITING)
        {
                client = server.available();
                if (!client) return;
                Serial.println("Got client");
                requestStatus = RECEIVING_HEADERS;
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

























void comm_init() {
/******************************************************************************
 *                                 Station                                    *
 ******************************************************************************/
        
/******************************************************************************
 *                                  Server                                    *
 ******************************************************************************/

/******************************************************************************
 *                                  Client                                    *
 ******************************************************************************/

}