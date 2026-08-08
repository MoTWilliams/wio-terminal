#ifndef RECORDER_H
#define RECORDER_H

#include <Arduino.h>
#include "audioBuffer.h"

class Wav {
public:
        static bool writeHeader();

        static constexpr size_t sampleRate();
        static constexpr size_t numSamples();
private:
        static constexpr uint32_t SAMPLE_RATE = 1600;
        static constexpr uint32_t DURATION_SEC = 3;
        static constexpr uint16_t NUM_CHANNELS = 1;
        static constexpr uint16_t BITS_PER_SAMPLE = 16;

        static constexpr uint16_t AUDIO_FORMAT = 1; // PCM
        static constexpr uint32_t FORMAT_INFOCHUNK_SIZE = 16;

        static constexpr uint32_t NUM_SAMPLES = SAMPLE_RATE * DURATION_SEC;
        static constexpr uint16_t BYTES_PER_SAMPLE = BITS_PER_SAMPLE / 8;
        static constexpr uint16_t BLOCK_ALIGN = NUM_CHANNELS * BYTES_PER_SAMPLE;
        static constexpr uint32_t BYTE_RATE = SAMPLE_RATE * BLOCK_ALIGN;
        static constexpr uint32_t DATA_SIZE = NUM_SAMPLES * BLOCK_ALIGN;
        static constexpr uint32_t CHUNK_SIZE = 36 + DATA_SIZE;
};

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

        AudioBuffer buffer;

        void update_recording();
        void update_recordingDone();
        void update_sendingFile();
};

#endif