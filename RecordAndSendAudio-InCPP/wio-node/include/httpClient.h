#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <WiFiClient.h>
#include "httpConnection.h"
#include "lineBuffer.h"

class HTTPClient {
public:
        HTTPClient();

        void update(unsigned long now);

        bool POST(const char *path);
private:
        WiFiClient client;
        LineBuffer lineBuffer;
        HTTPConnection connection;

        enum class Status {
                IDLE,
                CONNECTING,
                SENDING,
                WAITING_FOR_RESPONSE,
                READING_RESPONSE_HEADER,
                READING_RESPONSE_BODY
        };

        Status status = Status::IDLE;

        static constexpr size_t PATH_BUFFER_SIZE = 64 + 1;
        char path_[PATH_BUFFER_SIZE] = {0};

        unsigned long lastChecked = 0;
        int tries = 0;
        unsigned long responseWaitStarted = 0;
        size_t bytesToRead = 0;

        void reset();

        void update_connecting(unsigned long now);
        void update_sending(unsigned long now);
        void update_waitingForResponse(unsigned long now);
        void update_readingResponseHeader(unsigned long now);
        void update_readingResponseBody(unsigned long now);
};

#endif