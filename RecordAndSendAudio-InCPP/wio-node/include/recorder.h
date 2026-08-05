#ifndef RECORDER_H
#define RECORDER_H

class Recorder {
public:
        Recorder();

        void begin();
        void update();

        bool startRecording();
private:
        enum class Status { IDLE, RECORDING, RECORDING_DONE, SENDING_FILE };

        Status status = Status::IDLE;
        unsigned long started = 0;

        void update_recording();
        void update_recordingDone();
        void update_sendingFile();
};

#endif