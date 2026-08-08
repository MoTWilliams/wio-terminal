#include "audioBuffer.h"

// Returns false if a buffer is filled
void AudioBuffer::append(int16_t sample) {
        buffer[i][j++] = sample;

        if (j == SAMPLES_PER_BUFFER)
        {
                readyIndex = i;
                ready[i] = true;
                i = (i + 1) % 2;
                j = 0;
        }
}

size_t AudioBuffer::retrieveContents(int16_t* out) {        
        if (lastChunk)
        {
                // Last chunk falls on chunk boundary
                if (j == 0) { lastChunk = false; return 0; }

                memcpy(out, buffer[i], j * sizeof(int16_t));

                size_t samplesRetrieved = j;
                j = 0;
                lastChunk = false;

                return samplesRetrieved;
        }
        
        if (!ready[readyIndex]) return 0;

        memcpy(out, buffer[readyIndex], SAMPLES_PER_BUFFER * sizeof(int16_t));
        
        ready[readyIndex] = false;
        
        return SAMPLES_PER_BUFFER;
}

bool AudioBuffer::isReady() { return ready[readyIndex]; }

void AudioBuffer::signalLastChunk() {
        lastChunk = true;
}