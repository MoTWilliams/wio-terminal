#ifndef RECORDER_H
#define RECORDER_H

class HTTPClient;

class Recorder {
public:
        Recorder(HTTPClient &client);

        void begin();
        void update(unsigned long now);

        void startRecording(unsigned long now);
private:
        HTTPClient &httpClient;
        
        enum class Status : uint8_t { IDLE, RECORDING, FINISHING };

        Status status = Status::IDLE;
        unsigned long startedRecording = 0;

        void update_recording(unsigned long now);
        void update_finishing(unsigned long now);
};

#endif