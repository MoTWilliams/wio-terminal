#include <Arduino.h>
#include "wifiStation.h"
#include "connectionMonitor.h"
#include "recorder.h"
#include "secrets.h"
#include "httpServer.h"

HTTPServer::HTTPServer(
                WiFiStation &station,
                ConnectionMonitor &monitor,
                Recorder &recorder
        ) : wifiStation(station), connectionMonitor(monitor), 
          server(Secrets::port()), recorder(recorder), 
          connection(connectionMonitor, client, lineBuffer) {}

void HTTPServer::begin() {
        server.begin();
}

/******************************************************************************/

void HTTPServer::update(unsigned long now) {
        // No network actions if there's no connection
        if (wifiStation.isOffline()) return;

        switch (status)
        {
                case Status::LISTENING: 
                        update_listening(now); return;
                case Status::READING_REQUEST_LINE:
                        update_readingRequestLine(now); return;
                case Status::READING_HEADERS:
                        update_readingHeaders(now); return;
                case Status::HANDLING_REQUEST:
                        update_handlingRequest(now); return;
                case Status::SENDING_RESPONSE:
                        update_sendingResponse(now); return;
        }
}

void HTTPServer::update_listening(unsigned long now) {
        client = server.available();
        if (!client) return;    // No bytes to read

        connection.updateLastProgress();
        status = Status::READING_REQUEST_LINE;
}

void HTTPServer::update_readingRequestLine(unsigned long now) {
        char c = '\0';
        if (!connection.readChar(c)) return;

        if (!connection.processChar(c))
        {
                if (lineBuffer.didOverflow()) reset();
                return;
        }

        // Discard line ending and slice method and path
        const char* methodText = lineBuffer.data();
        char* path = strchr(methodText, ' ');

        if (path == nullptr) { reset(); return; }

        *path = '\0';   // Null-terminate method
        path++;         // Begin path

        // Null-terminate path and discard the rest of the line
        char *endLine = strchr(path, ' ');

        if (endLine == nullptr) { reset(); return; }

        *endLine = '\0';
        endLine++;

        // Store method and path (as an action)
        connection.setMethod(methodText);
        connection.setAction(path);

        // Clear buffer and move on
        lineBuffer.clear();
        status = Status::READING_HEADERS;
}

void HTTPServer::update_readingHeaders(unsigned long now) {
        char c = '\0';
        // if (!readChar(c)) return;
        if (!connection.readChar(c)) return;

        if (!connection.processChar(c))
        {
                if (lineBuffer.didOverflow()) reset();
                return;
        }

        // Line end reached. Discard and check for empty last line
        if (lineBuffer.length() == 0)
        {
                lineBuffer.clear();
                status = Status::HANDLING_REQUEST;
                return;
        }

        // Discard non-empty line and keep reading
        lineBuffer.clear();
}

void HTTPServer::update_handlingRequest(unsigned long now) {
        // Eventually set a flag or something, maybe, to make this more generic
        if (connection.recordingRequested()) recorder.startRecording(now);

        status = Status::SENDING_RESPONSE;
}

void HTTPServer::update_sendingResponse(unsigned long now) {
        if(!buildResponse(
                connection.method(), connection.action()
        )) { reset(); return; }

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.print("Content-Length: ");
        client.println(lineBuffer.length());
        client.println("Connection: close");
        client.println();
        client.print(lineBuffer.c_str());

        reset();
}

/******************************************************************************/

void HTTPServer::reset() {
        connection.reset();

        status = Status::LISTENING;
}

bool HTTPServer::buildResponse(const char* methodStr, const char* actionStr) {
        lineBuffer.clear();
        
        const char *p = methodStr;
        while (*p != '\0')
        {
                if (!lineBuffer.appendChar(*p)) return false;
                p++;
        }

        if (!lineBuffer.appendChar(' ')) return false;
        if (!lineBuffer.appendChar('(')) return false;

        p = actionStr;
        while (*p != '\0')
        {
                if (!lineBuffer.appendChar(*p)) return false;
                p++;
        }

        if (!lineBuffer.appendChar(')')) return false;
        return true;
}
