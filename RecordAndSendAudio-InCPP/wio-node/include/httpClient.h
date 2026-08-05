#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <WiFiClient.h>
#include "httpConnection.h"
#include "lineBuffer.h"

class HTTPClient {
public:
        HTTPClient();

        void update();

        bool POST(const char *path);

        // This can also be used to transmit, for example, a log file
        bool POST(const char *path, const char *fileName);

        bool isBusy();
private:
        WiFiClient client;
        LineBuffer lineBuffer;
        HTTPConnection connection;

        enum class Status {
                IDLE,
                CONNECTING,
                SENDING,
                SENDING_HEADERS,
                SENDING_BODY,
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

        void update_connecting();
        void update_sending();
        void update_sendingHeaders();
        void update_sendingBody();
        void update_waitingForResponse();
        void update_readingResponseHeader();
        void update_readingResponseBody();
};

#endif