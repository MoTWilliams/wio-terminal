#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <WiFiClient.h>
class ConnectionMonitor;
class LineBuffer;

class HTTPConnection {
public:
        HTTPConnection(
                ConnectionMonitor &monitor, 
                WiFiClient &client, LineBuffer &buffer);

        void reset();
        void updateLastProgress();

        void setMethod(const char* method);
        void setAction(const char* path);

        const char* method() const;
        const char* action() const;

        bool readChar(char &c);
        bool processChar(char c);

        bool recordingRequested();
private:
        ConnectionMonitor &connectionMonitor;
        WiFiClient &client;
        
        enum class Method : uint8_t { UNKNOWN, GET, POST };
        enum class Action : uint8_t {
                NONE,
                RECORD,
                RECORDING_STARTED,
                RECORDING_STOPPED,
                STATUS
        };

        Method method_ = Method::UNKNOWN;
        Action action_ = Action::NONE;

        LineBuffer &lineBuffer;
        unsigned long lastProgress = 0;
};

#endif