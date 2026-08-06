#include <Arduino.h>
#include <rpcWiFi.h>
#include "systemObjects.h"
#include "secrets.h"
#include "paths.h"
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

        connection.setMethod("POST");

        snprintf(path_, ENDPOINT_BUFFER_SIZE, "%s", path);

        // File should not be empty
        if (!strcmp(path, Paths::ENDPOINT_FILE_SEND))
        {
                if (!System::sdCard.file_openToRead()) 
                        { reset(); return false; }
                
                contentLength = System::sdCard.file_size();
                if (contentLength == 0) { reset(); return false; }
        }
        
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
                case Status::SENDING_HEADERS:
                        update_sendingHeaders(); return;
                case Status::SENDING_BODY:
                        update_sendingBody(); return;
                case Status::WAITING_FOR_RESPONSE:
                        update_waitingForResponse(); return;
                case Status::READING_RESPONSE_HEADER:
                        update_readingResponseHeader(); return;
                case Status::READING_RESPONSE_BODY:
                        update_readingResponseBody(); return;
                
        }
}

void HTTPClient::update_connecting() {
        if (tries != 0 &&
            !(tries < MAX_TRIES && System::now - lastChecked >= RETRY_INTERVAL))
                { reset(); return; }

        lastChecked = System::now;

        if (!client.connect(Secrets::host(), Secrets::port()))
                { tries++; return; }
        
        tries = 0;
        status = Status::SENDING_HEADERS;
}

void HTTPClient::update_sendingHeaders() {
        if (!System::connectionMonitor.checkConnection(client))
                { reset(); return; }

        // Send POST /recording/stopped with no body
        client.printf("%s %s HTTP/1.1\r\n", connection.method(), path_);
        client.printf("Host: %s\r\n", Secrets::host());

        if (!strcmp(path_, Paths::ENDPOINT_FILE_SEND))
                client.println("Content-Type: application/octet-stream");
        else client.println("Content-Type: text/plain");
        
        client.printf("Content-Length: %d\r\n", contentLength);
        client.println("Connection: close");

        // End headers
        client.println();

        // Move on to sending the body for non-empty POST requests
        if (!strcmp(path_, Paths::ENDPOINT_FILE_SEND))
        {
                status = Status::SENDING_BODY;
                return;
        }

        // Otherwise, clear the buffer and get ready to receive response
        lineBuffer.clear();
        responseWaitStarted = System::now;
        status = Status::WAITING_FOR_RESPONSE;
}

void HTTPClient::update_sendingBody() {
        if (!System::connectionMonitor.checkConnection(client))
                { reset(); return; }

        // Body finished. Close the file and get ready to receive response
        if (System::sdCard.file_EOF())
        {
                System::sdCard.file_close();
                lineBuffer.clear();
                responseWaitStarted = System::now;
                status = Status::WAITING_FOR_RESPONSE;
                return;
        }

        // Retrieve next chunk
        uint8_t buffer[System::sdCard.FILE_CHUNK_SIZE] = {0};

        size_t bytesRead = System::sdCard.file_readChunk(buffer);
        if (bytesRead == 0)
        {
                System::sdCard.file_close();
                reset();
                return;
        }

        // Send the chunk. Write doesn't always send the full number of bytes,
        // so continue until the whole chunk is sent
        size_t bytesSent = 0;

        while (bytesSent < bytesRead)
        {
                size_t sent = client.write(
                        buffer + bytesSent, bytesRead - bytesSent
                );

                if (sent == 0)
                {
                        Serial.println("Send chunk (of chunk) failed");
                        System::sdCard.file_close();
                        reset();
                        return;
                }

                bytesSent += sent;
        }
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
        // discard the response body. Reset on successful completion
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

        path_[0] = '\0';
        tries = 0;
        contentLength = 0;
        bytesToRead = 0;

        status = Status::IDLE;
}
