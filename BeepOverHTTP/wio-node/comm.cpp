// https://wiki.seeedstudio.com/Wio-Terminal-Wi-Fi/#configure-as-ap-mode-simple-web-server-example-code
// https://wiki.seeedstudio.com/Wio-Terminal-Wi-Fi/#wi-fi-incomingClient-example-code
#include <Arduino.h>
#include <string.h>
#include <stdbool.h>
#include <rpcWiFi.h>
#include <WiFiClient.h>
#include "comm.h"
#include "logging.h"
#include "arduino_secrets.h"
#include "buzzer.h"
#include "buffer.h"



/******************************************************************************
 *                                  Server                                    *
 ******************************************************************************/
typedef enum {
        LISTENING,
        RECEIVING_HEADERS,
        RECEIVING_BODY,
        RESPONDING
} ServerStatus;

static WiFiServer server(HA_PORT);
static WiFiClient incomingClient;
static ServerStatus serverStatus = LISTENING;

void server_init(void) {
        server.begin();
}

void server_update(void) {
        if (serverStatus == LISTENING)
        {
                incomingClient = server.available();
                if (!incomingClient) return;
                Serial.println("Got incomingClient");
                serverStatus = RECEIVING_HEADERS;
                return;
        }

        static int bodyLen;
        static int readBytes = 0;
        static char lineBuffer[512 + 1] = {0};

        if (serverStatus == RECEIVING_HEADERS)
        {
                if (!incomingClient.connected())
                // Or if too long with no activity (add later)
                {
                        incomingClient.stop();
                        serverStatus = LISTENING;
                        return;
                }

                if (!incomingClient.available()) return;

                // Parse headers
                static char c, *p = lineBuffer;

                // Later, change c to an int (see clientStatus == READING_RESPONSE)
                c = incomingClient.read();

                if (c == '\n')
                {
                        if (strlen(lineBuffer) == 0)
                        {
                                Serial.println("Blank line detected. End of header");
                                Serial.println();
                                serverStatus = RECEIVING_BODY;
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

        if (serverStatus == RECEIVING_BODY)
        {
                static char *p;

                if (readBytes == 0)
                {
                        p = lineBuffer;
                        *p = '\0';
                }

                if (readBytes < bodyLen) 
                {
                        if (!incomingClient.available()) return;

                        *p = incomingClient.read();
                        Serial.write(*p);
                        p++;
                        *p = '\0';
                        readBytes++;
                }

                if (readBytes == bodyLen)
                {
                        serverStatus = RESPONDING;
                        return;
                }
        }

        if (serverStatus == RESPONDING)
        {
                Serial.printf("\nbody = \"%s\"\n\n", lineBuffer);
                if (strcmp(lineBuffer, "beep") == 0) buzzer_beep();
                
                incomingClient.println("HTTP/1.1 200 OK");
                incomingClient.println("Content-Type: test/plain");
                incomingClient.println("Connection: close");
                incomingClient.println();
                incomingClient.print("Wio: Beep request handled");
                incomingClient.println();

                incomingClient.stop();
                serverStatus = LISTENING;
                return;
        }
}

/******************************************************************************
 *                                  Client                                    *
 ******************************************************************************/

typedef enum {
        IDLE,
        CONNECTING,
        SENDING,
        WAITING_FOR_RESPONSE,
        READING_RESPONSE_HEADER,
        READING_RESPONSE_BODY
} ClientState;

static WiFiClient outgoingClient;
static ClientState clientStatus = IDLE;

static const size_t METHOD_BUFFER_SIZE = 16 + 1;
static const size_t PATH_BUFFER_SIZE = 64 + 1;
static const size_t LINE_BUFFER_SIZE = 128 + 1;
static const size_t BODY_BUFFER_SIZE = 1024 + 1;

static char methodStorage[METHOD_BUFFER_SIZE] = {0};
static char pathStorage[PATH_BUFFER_SIZE] = {0};
static char headerLineStorage[LINE_BUFFER_SIZE] = {0};
static char bodyStorage[BODY_BUFFER_SIZE] = {0};

static Buffer method = buffer_init(methodStorage, METHOD_BUFFER_SIZE);
static Buffer path = buffer_init(pathStorage, PATH_BUFFER_SIZE);
static Buffer headerLine = buffer_init(headerLineStorage, LINE_BUFFER_SIZE);
static Buffer body = buffer_init(bodyStorage, BODY_BUFFER_SIZE);

static size_t bodyBytesToRead = 0;
static size_t bytesRead = 0;

static void resetClientState(bool resetAll) {
        outgoingClient.stop();
        
        buffer_clear(&method, resetAll);
        buffer_clear(&path, resetAll);
        buffer_clear(&headerLine, resetAll);
        buffer_clear(&body, resetAll);

        bodyBytesToRead = 0;
        bytesRead = 0;

        clientStatus = IDLE;
}

/* 
 * m must be non-NULL. m is the method code (GET, POST, etc.), p is the path, 
 * and b is the body payload 
 */
static bool buildRequest(const char *m, const char *p, const char *b) {
        // Reject if client is busy. Later, requests can be added to a queue
        if (clientStatus != IDLE)
        {
                Serial.print(m);
                Serial.println(" failed: outgoingClient busy");
                return false;
        }

        // Method code is required
        if (!m) return false;
        if (!buffer_copyFromCString(&method, m)) return false;

        // Path is optional
        if (!p)
        {
                if (!buffer_copyFromCString(&path, "/")) return false;
        }
        else
        {
                if (!buffer_copyFromCString(&path, p)) return false;
        }

        // Body text is also optional
        if (!b) return true;
        if (!buffer_copyFromCString(&body, b)) return false;
        return true;
}

void client_POST(const char *path, const char *body) {
        if (!buildRequest("POST", path, body))
        {
                resetClientState(true);
                Serial.println("POST request failed");
                return;
        }

        clientStatus = CONNECTING;
}

void client_GET(const char *path) {
        if (!buildRequest("GET", path, NULL))
        {
                resetClientState(true);
                Serial.println("GET request failed");
                return;
        }

        clientStatus = CONNECTING;
}

void client_update(void) {
        // No client requests in progress
        if (clientStatus == IDLE) return;

        unsigned long now = millis();
        static unsigned long responseWaitStarted;

        static const unsigned long TIMEOUT = 3000;
        static unsigned long lastProgress = now;

        if (clientStatus == CONNECTING)
        {
                Serial.printf("Attempting %s request...", method.data);
                Serial.println();
                
                static const unsigned long RETRY_INTERVAL = 3000;
                static const int MAX_TRIES = 3;

                static unsigned long lastChecked;
                static int tries = 0;

                if (tries == 0 || 
                    (tries < MAX_TRIES && now - lastChecked >= RETRY_INTERVAL))
                {
                        Serial.printf("Connecting to %s...", HOST);
                        Serial.println();

                        lastChecked = now;
                        if (outgoingClient.connect(HOST, HA_PORT))
                        {
                                tries = 0;
                                clientStatus = SENDING;
                                return;
                        }

                        // keep retrying
                        tries++;
                        return;
                }

                // Failed too many times
                Serial.printf("%s request not sent. ", method.data);
                Serial.println("Failed to connect");

                resetClientState(true);

                tries = 0;
                clientStatus = IDLE;
                return;
        }

        if (clientStatus == SENDING)
        {
                if (!outgoingClient.connected())
                {
                        Serial.printf("%s request not sent. ", method.data);
                        Serial.println("Connection lost");

                        resetClientState(true);
                        return;
                }
                
                outgoingClient.printf(
                        "%s %s HTTP/1.1\r\n", method.data, path.data);
                outgoingClient.printf("Host: %s\r\n", HOST);
                outgoingClient.println("Connection: close");

                // Send body metadata for POST requests
                if (strcmp(method.data, "POST") == 0)
                {
                        outgoingClient.println("Content-Type: text/plain");
                        outgoingClient.print("Content-Length: ");
                        outgoingClient.println(body.length);
                }

                // Empty line after headers
                outgoingClient.println();

                // Send body, if it exists
                if (body.length > 0) 
                        outgoingClient.write(body.data, body.length);

                // Sent. Clear header and body buffers in preparation
                buffer_clear(&headerLine, false);
                buffer_clear(&body, false);
                bodyBytesToRead = 0;
                responseWaitStarted = now;
                clientStatus = WAITING_FOR_RESPONSE;
                return;
        }

        if (clientStatus == WAITING_FOR_RESPONSE)
        {
                if (!outgoingClient.connected())
                {
                        Serial.printf("No response received. ");
                        Serial.println("Connection lost");

                        resetClientState(true);
                        return;
                }
                
                static const unsigned long TIMEOUT_INTERVAL = 1000;

                // Check continuously until timeout (1 second)
                if (outgoingClient.available())
                {
                        lastProgress = now;
                        clientStatus = READING_RESPONSE_HEADER;
                        return;
                }

                // Timeout--reset and discard request. Later, add it to queue
                if (now - responseWaitStarted >= TIMEOUT_INTERVAL)
                {
                        Serial.printf("%s request timeout. ", method.data);
                        Serial.println("No request received");

                        resetClientState(true);
                        return;
                }

                // Keep retrying
                return;
        }

        if (clientStatus == READING_RESPONSE_HEADER)
        {               
                if (!outgoingClient.connected())
                {
                        Serial.printf("Read response header failed. ");
                        Serial.println("Connection lost");

                        resetClientState(true);
                        return;
                }

                // Timeout if no bytes received for 3 seconds
                if (!outgoingClient.available())
                        if (now - lastProgress >= TIMEOUT)
                        {
                                resetClientState(true);
                                return;
                        // Out of bytes but not yet timed out
                        } else return;
                
                int cVal = outgoingClient.read();
                // read() didn't actually return a byte
                if (cVal < 0) { resetClientState(true); return; }
                lastProgress = now;

                char c = (char)cVal;
                
                // Toss CR. The next byte should be \n
                if (c == '\r') return;

                // Process completed line
                if (c == '\n')
                {
                        Serial.println(headerLine.data);

                        // Blank line seen. Header finished
                        if (headerLine.length == 0)
                        {
                                lastProgress = now;
                                clientStatus = READING_RESPONSE_BODY;
                                return;
                        }

                        // Check for and parse body length
                        const char *goal = "Content-Length: ";
                        size_t len = strlen(goal);
                        
                        if (strncmp(headerLine.data, goal, len) == 0)
                        {
                                // Just pass a pointer to the beginning of the 
                                // number into strtoul
                                const char *numStart = headerLine.data + len;
                                bodyBytesToRead = (size_t)strtoul(
                                        numStart, NULL, 0
                                );
                        }

                        buffer_clear(&headerLine, false);
                        return;
                }

                // Anything else
                if (!buffer_appendChar(&headerLine, c))
                {
                        Serial.printf("Append \'%c\' failed. Read: ", c);
                        Serial.println(headerLine.data);
                        resetClientState(true);
                        return;
                }
                return;
        }

        if (clientStatus == READING_RESPONSE_BODY)
        {
                // Finished reading
                if (bytesRead >= bodyBytesToRead)
                {
                        Serial.print("Response body: ");
                        Serial.println(body.data);

                        bytesRead = 0;
                        resetClientState(false);
                        return;
                }

                if (!outgoingClient.connected())
                {
                        Serial.printf("Read response body failed. ");
                        Serial.println("Connection lost");

                        resetClientState(true);
                        return;
                }

                // Timeout if no bytes received for 3 seconds
                if (!outgoingClient.available())
                        if (now - lastProgress >= TIMEOUT)
                        {
                                resetClientState(true);
                                return;
                        // Out of bytes but not yet timed out
                        } else return;
                
                // Append next byte
                int cVal = outgoingClient.read();
                // read() didn't actually return a byte
                if (cVal < 0) { resetClientState(true); return; }
                char c = (char)cVal;
                lastProgress = now;

                if (!buffer_appendChar(&body, c))
                {
                        Serial.printf("Append \'%c\' failed. Read:", c);
                        Serial.println(body.data);
                        resetClientState(true);
                        return;
                }
                bytesRead++;
                return;
        }
}

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
