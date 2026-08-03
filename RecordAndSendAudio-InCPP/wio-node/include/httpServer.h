#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <WiFiClient.h>
#include <rpcWiFi.h>
#include "httpConnection.h"
#include "lineBuffer.h"

class Recorder;

class HTTPServer {
public:
        HTTPServer();

        void begin();
        void update(unsigned long now);
private:
        WiFiServer server;
        WiFiClient client;
        LineBuffer lineBuffer;
        HTTPConnection connection;
        
        enum class Status {
                LISTENING,
                READING_REQUEST_LINE,
                READING_HEADERS,
                HANDLING_REQUEST,
                SENDING_RESPONSE
        };

        Status status = Status::LISTENING;

        void reset();

        bool buildResponse(const char* method, const char* action);

        void update_listening(unsigned long now);
        void update_readingRequestLine(unsigned long now);
        void update_readingHeaders(unsigned long now);
        void update_handlingRequest(unsigned long now);
        void update_sendingResponse(unsigned long now);
};

#endif