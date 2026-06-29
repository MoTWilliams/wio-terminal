#include <Arduino.h>
#include <rpcWiFi.h>
#include <WiFiClient.h>
#include <stdbool.h>
#include "network.h"
#include "arduino_secrets.h"
#include "recorder.h"

typedef enum {
        LISTENING,
        READING_REQUEST_LINE,
        READING_HEADERS,
        HANDLING_REQUEST,
        SENDING_RESPONSE
} ServerStatus;

typedef enum {
        NONE,
        RECORD,
        STATUS  // Not used in this prototype
} Action;

static ServerStatus status = LISTENING;
static Action action = NONE;

static WiFiServer server(HA_PORT);
static WiFiClient client;

void server_init(void) {
        server.begin();
}

static const size_t LINE_BUFFER_SIZE = 512 + 1;
static char lineBuffer[LINE_BUFFER_SIZE] = {0};
static char *p = lineBuffer;
static size_t lineLength = 0;

typedef enum {
        UNKNOWN,
        GET,
        POST
} Method;

static Method method = UNKNOWN;

static const char *getMethod(void) {
        switch (method)
        {
                case GET: return "GET";
                case POST: return "POST";
                default: return "UNKNOWN";
        }
}

static const char *getAction(void) {
        switch (action)
        {
                case RECORD: return "Record 3 seconds";
                case STATUS: return "Report system status";
                default: return "Action not handled";
        }
}

static void resetServer(void) {
        client.stop();
        status = LISTENING;

        lineBuffer[0] = '\0';
        p = lineBuffer;
        lineLength = 0;

        method = UNKNOWN;
        action = NONE;
}

static unsigned long lastProgress = 0;

void server_update(void) {
        // Don't touch network actions if there's no active connection
        if (wifiStation_isOffline()) return;

        if (status == LISTENING)
        {
                client = server.available();
                if (!client) return;    // No bytes to read
                
                lastProgress = millis();
                status = READING_REQUEST_LINE;
                return;
        }

        if (status == READING_REQUEST_LINE)
        {
                // Ensure there's still an active connection
                if (!wifiStation_checkClientConnection(client, &lastProgress))
                {
                        resetServer();
                        return;
                }

                // No bytes to read this pass
                if (!client.available()) return;

                // Store the next char in the buffer
                int cVal = client.read();
                if (cVal < 0) 
                { 
                        resetServer();
                        wifiStation_handleClientConnectionLost(client);
                        return;
                }
                
                lastProgress = millis();
                char c = (char)cVal;

                if (c == '\r') return;  // Discard CR

                if (c != '\n')          // Gather chars until end of line
                {
                        if (lineLength >= LINE_BUFFER_SIZE - 1)
                        {
                                Serial.print("lineBuffer overflow. ");
                                Serial.print("Truncated to: ");
                                Serial.println(lineBuffer);

                                resetServer();
                                return;
                        }
                        *p++ = c;
                        *p = '\0';
                        lineLength++;
                        return;
                }

                // Discard line ending. Parse method and path into slices
                char *methodText = lineBuffer;
                char *path = strchr(methodText, ' ');

                if (path == NULL)
                {
                        Serial.print("Malformed request line: ");
                        Serial.println(lineBuffer);

                        resetServer();
                        return;
                }

                *path = '\0';   // Null-terminate method
                path++;         // Begin path slice

                // Null-terminate path and discard the rest of the line
                char *endLine = strchr(path, ' ');

                if (endLine == NULL)
                {
                        Serial.print("Malformed request line: ");
                        Serial.println(lineBuffer);

                        resetServer();
                        return;
                }

                *endLine = '\0';
                endLine++;

                Serial.printf("%s %s %s", methodText, path, endLine);
                Serial.println();

                // Store method and path/command
                if (!strcmp(methodText, "GET")) method = GET;
                else if (!strcmp(methodText, "POST")) method = POST;
                else method = UNKNOWN;

                // This should eventually handle the path more explicitly, but
                // this prototype only needs to handle receiving POST
                // /record/start requests, so we're assuming every POST is a
                // start request
                if (method == POST)
                {
                        action = RECORD;
                }
                else
                {
                        Serial.print(getMethod());
                        Serial.println(" method not handled");
                }

                // Clear buffer and move on to headers
                lineBuffer[0] = '\0';
                p = lineBuffer;
                lineLength = 0;
                status = READING_HEADERS;
                return;
        }

        if (status == READING_HEADERS)
        {
                if (!wifiStation_checkClientConnection(client, &lastProgress))
                {
                        resetServer();
                        return;
                }

                // No bytes to read this pass
                if (!client.available()) return;

                // Store the next char in the buffer
                int cVal = client.read();
                if (cVal < 0) 
                { 
                        resetServer();
                        wifiStation_handleClientConnectionLost(client);
                        return;
                }
                
                lastProgress = millis();
                char c = (char)cVal;

                if (c == '\r') return;

                if (c != '\n')
                {
                        if (lineLength >= LINE_BUFFER_SIZE - 1)
                        {
                                Serial.print("lineBuffer overflow. ");
                                Serial.print("Truncated to: ");
                                Serial.println(lineBuffer);

                                resetServer();
                                return;
                        }
                        *p++ = c;
                        *p = '\0';
                        lineLength++;
                        return;
                }

                if (strlen(lineBuffer) == 0)
                {
                        // Clear buffers and send response
                        lineBuffer[0] = '\0';
                        p = lineBuffer;
                        lineLength = 0;
                        status = HANDLING_REQUEST;
                        return;
                }

                // Discard non-empty header line and keep reading
                lineBuffer[0] = '\0';
                p = lineBuffer;
                lineLength = 0;
                return;
        }

        if (status == HANDLING_REQUEST)
        {
                if (action == RECORD) recorder_startRecording();
                // Otherwise, no nothing and move on

                status = SENDING_RESPONSE;
                return;
        }

        if (status == SENDING_RESPONSE)
        {
                if (method == POST)
                        snprintf(
                                lineBuffer, LINE_BUFFER_SIZE, "%s (%s)", 
                                getMethod(), getAction()
                        );
                else
                        snprintf(
                                lineBuffer, LINE_BUFFER_SIZE, "%s", getMethod()
                        );
                
                size_t contentLength = strlen(lineBuffer);

                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: text/plain");
                client.print("Content-Length: ");
                client.println(contentLength);
                client.println("Connection: close");
                client.println();
                client.print(lineBuffer);

                resetServer();
                return;
        }
}