#include <Arduino.h>
#include <rpcWiFi.h>
#include "systemObjects.h"
#include "secrets.h"
#include "HTTPClient.h"

namespace {
        constexpr unsigned long RETRY_INTERVAL = 3000;
        constexpr int MAX_TRIES = 3;
        constexpr unsigned long RESPONSE_WAIT_TIMEOUT = 3000;
}

HTTPClient::HTTPClient() : connection(client, lineBuffer) {}

bool HTTPClient::POST(const char *path) {
        if (System::wifiStation.isOffline()) return false;
        if (status != Status::IDLE)
        {
                Serial.println("HTTPClient busy; POST ignored");
                return false;
        }

        snprintf(path_, PATH_BUFFER_SIZE, "%s", path);

        connection.setMethod("POST");
        status = Status::CONNECTING;

        return true;
}

bool HTTPClient::isBusy() {
        return status != Status::IDLE;
}

/******************************************************************************/

void HTTPClient::update() {
        // No network actions if there's no connection
        if (System::wifiStation.isOffline()) return;

        switch (status)
        {
                case Status::IDLE: return;
                case Status::CONNECTING:
                        update_connecting(); return;
                case Status::SENDING:
                        update_sending(); return;
                case Status::WAITING_FOR_RESPONSE:
                        update_waitingForResponse(); return;
                case Status::READING_RESPONSE_HEADER:
                        update_readingResponseHeader(); return;
                case Status::READING_RESPONSE_BODY:
                        update_readingResponseBody(); return;
                
        }
}

void HTTPClient::update_connecting() {
        if (tries == 0 || 
            (tries < MAX_TRIES && System::now - lastChecked >= RETRY_INTERVAL))
        {
                lastChecked = System::now;
                if (!client.connect(Secrets::host(), Secrets::port())) tries++;
                else { tries = 0; status = Status::SENDING; }
                return;
        }

        // Failed too many times
        reset();

        tries = 0;
        status = Status::IDLE;
}

// TODO: Split this state into headers and body, and handle nonempty POST
void HTTPClient::update_sending() {
        if (!System::connectionMonitor.checkConnection(client))
                { reset(); return; }

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
        responseWaitStarted = System::now;
        status = Status::WAITING_FOR_RESPONSE;
}

void HTTPClient::update_sendingHeaders() {

}

void HTTPClient::update_sendingBody() {

}

void HTTPClient::update_waitingForResponse() {
        if (!System::connectionMonitor.checkConnection(client))
                { reset(); return; }
        
        // Check continuously for bytes to read
        if (client.available())
        {
                connection.updateLastProgress();
                status = Status::READING_RESPONSE_HEADER;
                return;
        }
        if (System::now - responseWaitStarted < RESPONSE_WAIT_TIMEOUT) return;

        // Give up and reset after 3 seconds (later, add to retry queue)
        reset();
}

void HTTPClient::update_readingResponseHeader() {
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

void HTTPClient::update_readingResponseBody() {
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
