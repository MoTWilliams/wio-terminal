#ifndef AUDIO_BUFFER_H
#define AUDIO_BUFFER_H

#include <Arduino.h>

class AudioBuffer {
public:
        static const size_t SAMPLES_PER_BUFFER = 512;

        void append(int16_t sample);
        size_t retrieveContents(int16_t* out); // + 1, % 2
        bool isReady();
        void signalLastChunk();
private:
        size_t i = 0, j = 0;
        int16_t buffer[2][SAMPLES_PER_BUFFER] = {0};
        bool ready[2] = {false};
        size_t readyIndex = 0;
        bool lastChunk = false;
};

#endif