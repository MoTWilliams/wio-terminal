#include <Arduino.h>
#include "systemObjects.h"
#include "secrets.h"
#include "httpServer.h"

HTTPServer::HTTPServer() : server(Secrets::port()),
        connection(client, lineBuffer) {}

void HTTPServer::begin() {
        server.begin();
}

/******************************************************************************/

void HTTPServer::update(unsigned long now) {
        // No network actions if there's no connection
        if (System::wifiStation.isOffline()) return;

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
                case Status::CLOSING_CONNECTION:
                        update_closingConnection(now); return;
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

        // Store method and path (as an action).
        connection.setMethod(methodText);
        System::dispatcher.setPendingAction(path);

        // Clear buffer and move on
        lineBuffer.clear();
        status = Status::READING_HEADERS;
}

void HTTPServer::update_readingHeaders(unsigned long now) {
        char c = '\0';
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
        if(!buildResponse(
                connection.method(), System::dispatcher.pendingAction()
        )) { reset(); return; }

        status = Status::SENDING_RESPONSE;
}

void HTTPServer::update_sendingResponse(unsigned long now) {

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/plain");
        client.print("Content-Length: ");
        client.println(lineBuffer.length());
        client.println("Connection: close");
        client.println();
        client.print(lineBuffer.c_str());

        responseSentAt = now;
        status = Status::CLOSING_CONNECTION;
}

void HTTPServer::update_closingConnection(unsigned long now) {
        if (now - responseSentAt < 100) return;

        reset();
        System::dispatcher.performPendingAction();
}

/******************************************************************************/

void HTTPServer::reset() {
        connection.reset();

        status = Status::LISTENING;
}

bool HTTPServer::buildResponse(const char* methodStr, const char* actionStr) {
        lineBuffer.clear();
        
        if (!lineBuffer.append(methodStr) ||
            !lineBuffer.append(" received (") ||
            !lineBuffer.append(actionStr) ||
            !lineBuffer.appendChar(')')) return false;

        return true;
}
