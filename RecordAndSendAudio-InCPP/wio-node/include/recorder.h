#ifndef RECORDER_H
#define RECORDER_H

class Recorder {
public:
        Recorder();

        void begin();
        void update(unsigned long now);

        void startRecording(unsigned long now);
private:
        enum class Status { IDLE, RECORDING, FINISHING };

        Status status = Status::IDLE;
        unsigned long started = 0;

        void update_recording(unsigned long now);
        void update_finishing(unsigned long now);
};

#endif