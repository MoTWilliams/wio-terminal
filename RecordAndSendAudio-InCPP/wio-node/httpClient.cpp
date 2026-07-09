#include <Arduino.h>
#include <rpcWiFi.h>
#include "wifiStation.h"
#include "connectionMonitor.h"
#include "secrets.h"
#include "HTTPClient.h"

namespace {
        constexpr unsigned long RETRY_INTERVAL = 3000;
        constexpr int MAX_TRIES = 3;
        constexpr unsigned long RESPONSE_WAIT_TIMEOUT = 3000;
}

HTTPClient::HTTPClient(WiFiStation &station, ConnectionMonitor &monitor) 
        : wifiStation(station), connectionMonitor(monitor),
          connection(connectionMonitor, client, lineBuffer) {}

bool HTTPClient::POST(const char *path) {
        if (wifiStation.isOffline()) return false;
        if (status != Status::IDLE)
        {
                Serial.println("HTTPClient busy; POST ignored");
                return false;
        }

        snprintf(path_, PATH_BUFFER_SIZE, "%s", path);

        connection.setMethod("POST");
        connection.setAction(path);
        status = Status::CONNECTING;

        return true;
}

/******************************************************************************/

void HTTPClient::update(unsigned long now) {
        // No network actions if there's no connection
        if (wifiStation.isOffline()) return;

        switch (status)
        {
                case Status::IDLE: return;
                case Status::CONNECTING:
                        update_connecting(now); return;
                case Status::SENDING:
                        update_sending(now); return;
                case Status::WAITING_FOR_RESPONSE:
                        update_waitingForResponse(now); return;
                case Status::READING_RESPONSE_HEADER:
                        update_readingResponseHeader(now); return;
                case Status::READING_RESPONSE_BODY:
                        update_readingResponseBody(now); return;
        }
}

void HTTPClient::update_connecting(unsigned long now) {
        if (tries == 0 || 
            (tries < MAX_TRIES && now - lastChecked >= RETRY_INTERVAL))
        {
                lastChecked = now;
                if (!client.connect(Secrets::host(), Secrets::port())) tries++;
                else { tries = 0; status = Status::SENDING; }
                return;
        }

        // Failed too many times
        reset();

        tries = 0;
        status = Status::IDLE;
}

void HTTPClient::update_sending(unsigned long now) {
        if (!connectionMonitor.checkConnection(client)) { reset(); return; }

        // Send POST /recording/stopped with no body
        client.printf("%s %s HTTP/1.1\r\n",connection.method(), path_);
        client.printf("Host: %s\r\n", Secrets::host());
        client.println("Content-Type: text/plain");
        client.println("Content-Length: 0");
        client.println("Connection: close");

        // End headers
        client.println();

        // Sent. Ensure buffer is empty
        lineBuffer.clear();
        responseWaitStarted = now;
        status = Status::WAITING_FOR_RESPONSE;
}

void HTTPClient::update_waitingForResponse(unsigned long now) {
        if (!connectionMonitor.checkConnection(client))
                { reset(); return; }
        
        // Check continuously for bytes to read
        if (client.available())
        {
                connection.updateLastProgress();
                status = Status::READING_RESPONSE_HEADER;
                return;
        }
        if (now - responseWaitStarted < RESPONSE_WAIT_TIMEOUT) return;

        // Give up and reset after 3 seconds (later, add to retry queue)
        reset();
}

void HTTPClient::update_readingResponseHeader(unsigned long now) {
        char c = '\0';
        if (!connection.readChar(c)) return;

        if (!connection.processChar(c))
        {
                if (lineBuffer.didOverflow()) reset();
                return;
        }

        // Blank line. Header finished
        if (lineBuffer.length() == 0)
        {
                status = Status::READING_RESPONSE_BODY;
                return;
        }

        // Get body length
        const char* line = lineBuffer.c_str();
        const char* goal = "Content-Length: ";
        size_t goalLen = strlen(goal);

        if (!strncmp(line, goal, goalLen))
        {
                const char* bytesToReadStr = line + goalLen;
                bytesToRead = 
                        static_cast<size_t>(strtoul(bytesToReadStr,nullptr,0));
        }

        // Clear buffer before moving on to the next line
        lineBuffer.clear();
}

void HTTPClient::update_readingResponseBody(unsigned long now) {
        // If something breaks, I'll add a print statement here. For now, just
        // discard the response body
        if (lineBuffer.length() >= bytesToRead)
        {
                reset();
                return;
        }
        
        char c = '\0';
        if (!connection.readChar(c)) return;

        if (!connection.processChar(c))
        {
                if (lineBuffer.didOverflow()) reset();
                return;
        }
}

/******************************************************************************/

void HTTPClient::reset() {
        connection.reset();
        bytesToRead = 0;

        status = Status::IDLE;
}
