#include <Arduino.h>
#include "network.h"
#include <rpcWiFi.h>
#include <WiFiClient.h>
#include "arduino_secrets.h"
#include <string.h>

static WiFiClient client;

typedef enum {
        IDLE,
        CONNECTING,
        SENDING,
        WAITING_FOR_RESPONSE,
        READING_RESPONSE_HEADER,
        READING_RESPONSE_BODY
} ClientStatus;

typedef enum {
        UNKNOWN,
        GET,
        POST
} RequestMethod;

typedef enum {
        NONE,
        NOTIFY_STOPPED
} Action;

static ClientStatus status = IDLE;
static RequestMethod method = UNKNOWN;
static Action action = NONE;

static const size_t LINE_BUFFER_SIZE = 512 + 1;
static char lineBuffer[LINE_BUFFER_SIZE] = {0};
char *p = lineBuffer;

static size_t bytesToRead = 0;
static size_t bytesRead = 0;

static void resetLineBuffer(void) {
        lineBuffer[0] = '\0';
        p = lineBuffer;

        bytesRead = 0;
}

static void resetClient(void) {
        client.stop();

        resetLineBuffer();
        bytesToRead = 0;

        status = IDLE;
        method = UNKNOWN;
        action = NONE;
}

static const char *getMethod(void) {
        switch (method)
        {
                case GET: return "GET";
                case POST: return "POST";
                default: return "UNKNOWN";
        }
}

/* Only handles empty POST requests */
void client_POST(const char *path) {
        if (wifiStation_isOffline())
        {
                Serial.println("POST ABORTED: System is offline");
                return;
        }
        
        if (status != IDLE)
        {
                Serial.println("POST ABORTED: Outgoing client busy");
                return;
        }
        
        if (strcmp(path, "/recording/stopped") == 0) action = NOTIFY_STOPPED;
        else
        {
                Serial.print("POST ABORTED: ");
                Serial.print(path);
                Serial.println(" action not supported");
                return;
        }

        method = POST;
        status = CONNECTING;
}

void client_update(void) {
        if (wifiStation_isOffline()) return;

        if (status == IDLE) return;

        unsigned long now = millis();

        if (status == CONNECTING)
        {
                static const unsigned long RETRY_INTERVAL = 3000;
                static const int MAX_TRIES = 3;

                static unsigned long lastChecked;
                static int tries = 0;

                if (tries == 0 ||
                    (tries < MAX_TRIES && now - lastChecked >= RETRY_INTERVAL))
                {
                        lastChecked = now;
                        if (!client.connect(HOST, HA_PORT)) tries++;
                        else
                        {
                                tries = 0;
                                status = SENDING;
                        }
                        return;
                }

                // Failed too many times
                Serial.print(getMethod());
                Serial.println(" ABORTED: Failed to connect");

                resetClient();

                tries = 0;
                status = IDLE;
                return;
        }

        static unsigned long responseWaitStarted;

        if (status == SENDING)
        {
                if (!wifiStation_checkClientConnection(client, NULL))
                {
                        resetClient();
                        return;
                }

                // char *body = "Recording stopped";
                // int bodyLength = strlen(body);

                // POST /recording/stopped is all we're sending
                client.printf("%s /recording/stopped HTTP/1.1\r\n",getMethod());
                client.printf("Host: %s\r\n", HOST);
                client.println("Content-Type: text/plain");
                client.println("Content-Length: 0");
                // client.println(bodyLength);
                client.println("Connection: close");
                
                // End headers
                client.println();

                // Send body text
                // client.write(body, bodyLength);

                // Sent. Ensure buffer is set to initial state
                resetLineBuffer();
                responseWaitStarted = now;
                status = WAITING_FOR_RESPONSE;
                return;
        }

        static unsigned long lastProgress = now;

        if (status == WAITING_FOR_RESPONSE)
        {
                if (!wifiStation_checkClientConnection(client, &lastProgress))
                {
                        resetClient();
                        return;
                }

                // Check continuously until timeout (3 seconds)
                if (client.available())
                {
                        lastProgress = now;
                        status = READING_RESPONSE_HEADER;
                        return;
                }

                static const unsigned long RESPONSE_WAIT_TIMEOUT = 3000;
                if (now - responseWaitStarted < RESPONSE_WAIT_TIMEOUT) return;

                // Discard request and reset after timeout
                resetClient();
                return;
        }
        
        static const unsigned long NO_BYTES_TIMEOUT = 3000;
        
        if (status == READING_RESPONSE_HEADER)
        {
                if (!wifiStation_checkClientConnection(client, NULL))
                {
                        resetClient();
                        return;
                }

                // Timeout if no bytes received for 3 seconds. Keep spinning if
                // no available bytes but not yet timed out
                if (!client.available())
                {
                        if (now - lastProgress >= NO_BYTES_TIMEOUT) 
                                resetClient();
                        return;
                }

                // Attempt to read the next byte
                int cVal = client.read();
                if (cVal < 0)   // read() didn't actually return a byte
                { 
                        resetClient();
                        wifiStation_handleClientConnectionLost(client);
                        return;
                }

                char c = (char)cVal;
                lastProgress = now;

                if (c == '\r') return;  // Discard CR
                if (c == '\n')          // Completed line
                {
                        Serial.println(lineBuffer);

                        // Blank line reached. Header finished
                        if (strlen(lineBuffer) == 0)
                        {
                                status = READING_RESPONSE_BODY;
                                return;
                        }

                        // Get body length
                        const char *goal = "Content-Length: ";
                        size_t len = strlen(goal);

                        if (strncmp(lineBuffer, goal, len) == 0)
                        {
                                const char *num = lineBuffer + len;
                                bytesToRead = (size_t)strtoul(num, NULL, 0);
                        }

                        // Clear buffer in preparation for the next line
                        resetLineBuffer();
                        return;
                }

                // Any other character
                if (bytesRead >= LINE_BUFFER_SIZE - 1)  // Buffer overflow
                {
                        // Unlikely. Just fail and reset
                        Serial.print("POST ABORTED: ");
                        Serial.printf("Append \'%c\' failed. ", c);
                        Serial.print("Buffer overflow. Read: ");
                        Serial.println(lineBuffer);
                        resetClient();
                        return;
                }

                // Append the character
                *p++ = c;
                *p = '\0';
                bytesRead++;
        }

        if (status == READING_RESPONSE_BODY)
        {
                // Finished reading
                if (bytesRead >= bytesToRead)
                {
                        Serial.print("Response body: ");
                        Serial.println(lineBuffer);

                        resetClient();
                        return;
                }

                if (!wifiStation_checkClientConnection(client, NULL))
                {
                        resetClient();
                        return;
                }

                // Timeout if no bytes received for 3 seconds
                if (!client.available())
                {
                        if (now - lastProgress >= NO_BYTES_TIMEOUT) 
                                resetClient();
                        return;
                }

                // Attempt to read the next byte
                int cVal = client.read();
                if (cVal < 0)   // read() didn't actually return a byte
                { 
                        resetClient();
                        wifiStation_handleClientConnectionLost(client);
                        return;
                }

                // Read successful
                char c = (char)cVal;
                lastProgress = now;

                if (bytesRead >= LINE_BUFFER_SIZE - 1)  // Buffer overflow
                {
                        // Unlikely. Just fail and reset
                        Serial.print("POST ABORTED: ");
                        Serial.printf("Append \'%c\' failed. ", c);
                        Serial.print("Buffer overflow. Read: ");
                        Serial.println(lineBuffer);
                        resetClient();
                        return;
                }

                // Append the character
                *p++ = c;
                *p = '\0';
                bytesRead++;
        }
}