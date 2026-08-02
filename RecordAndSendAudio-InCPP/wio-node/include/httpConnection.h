#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <WiFiClient.h>
class LineBuffer;

class HTTPConnection {
public:
        HTTPConnection(WiFiClient &client, LineBuffer &buffer);

        void reset();
        void updateLastProgress();

        void setMethod(const char* method);
        // void setAction(const char* path);

        const char* method() const;
        // const char* action() const;

        bool readChar(char &c);
        bool processChar(char c);
private:
        WiFiClient &client;
        
        enum class Method { UNKNOWN, GET, POST };
        // enum class Action {
        //         NONE,
        //         RECORD,
        //         RECORDING_STARTED,
        //         RECORDING_STOPPED,
        //         STATUS
        // };

        Method method_ = Method::UNKNOWN;
        // Action action_ = Action::NONE;

        LineBuffer &lineBuffer;
        unsigned long lastProgress = 0;
};

#endif