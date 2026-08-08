#include <Arduino.h>

#include "systemObjects.h"
#include "paths.h"
#include "recorder.h"

namespace {
        constexpr uint16_t ON = 12000;
        constexpr uint16_t OFF = 0;
}

bool Wav::writeHeader() {
        if (!System::sdCard.file_append("RIFF")) return false;
        if (!System::sdCard.file_write(CHUNK_SIZE)) return false;
        if (!System::sdCard.file_append("WAVE")) return false;

        if (!System::sdCard.file_append("fmt ")) return false;
        if (!System::sdCard.file_write(FORMAT_INFOCHUNK_SIZE)) return false;
        if (!System::sdCard.file_write(AUDIO_FORMAT)) return false;
        if (!System::sdCard.file_write(NUM_CHANNELS)) return false;
        if (!System::sdCard.file_write(SAMPLE_RATE)) return false;
        if (!System::sdCard.file_write(BYTE_RATE)) return false;
        if (!System::sdCard.file_write(BLOCK_ALIGN)) return false;
        if (!System::sdCard.file_write(BITS_PER_SAMPLE)) return false;

        if (!System::sdCard.file_append("data")) return false;
        if (!System::sdCard.file_write(DATA_SIZE)) return false;
        return true;
}

constexpr size_t Wav::sampleRate() { return SAMPLE_RATE; }

constexpr size_t Wav::numSamples() { return NUM_SAMPLES; }

Recorder::Recorder() {}

void Recorder::begin() {}

/******************************************************************************/

void Recorder::update() {
        switch (status)
        {
                case Status::IDLE: return;
                case Status::RECORDING: update_recording(); return;
                case Status::RECORDING_DONE: update_recordingDone(); return;
                case Status::SENDING_FILE: update_sendingFile(); return;
        }
}

const int16_t generateSample(unsigned int samplesCaptured);
void Recorder::update_recording() {
        unsigned long elapsed = System::now - started;
        static const size_t totalSamples = Wav::numSamples();
        static size_t samplesCaptured = 0;

        int16_t out[AudioBuffer::SAMPLES_PER_BUFFER];

        // Write normal full buffer
        if (buffer.isReady())
        {
                size_t samplesRetrieved = buffer.retrieveContents(out);
                if (!samplesRetrieved || 
                        !System::sdCard.file_write(out, samplesRetrieved))
                {
                        Serial.println("Chunk retrieval failed");
                        System::sdCard.file_close();
                        samplesCaptured = 0;
                        status = Status::IDLE;
                        return;
                }
        }

        // Recording complete. Flush the final partial buffer and close the file
        if (samplesCaptured >= totalSamples)
        {
                buffer.signalLastChunk();

                size_t samplesRetrieved = buffer.retrieveContents(out);

                if (samplesRetrieved &&
                        !System::sdCard.file_write(out, samplesRetrieved))
                {
                        Serial.println("Final chunk write failed");
                        System::sdCard.file_close();
                        samplesCaptured = 0;
                        status = Status::IDLE;
                        return;
                }

                // Serial.printf("Recorded %.4f seconds\n", elapsed);
                
                samplesCaptured = 0;
                status = Status::RECORDING_DONE;
                return;
        }
        
        // Capture the next sample
        int16_t sample = generateSample(samplesCaptured);
        buffer.append(sample);
        samplesCaptured++;
}

const int16_t generateSample(unsigned int samplesCaptured) {
        static const size_t oneSecond = Wav::numSamples() / 3;
        static const size_t twoSeconds = oneSecond + oneSecond;

        static const size_t halfPeriodSamples = Wav::sampleRate() / (440 * 2);

        // Silence for the 2nd second
        if (samplesCaptured > oneSecond && samplesCaptured <= twoSeconds)
                return OFF;
        
        // Generate tone for the 1st and 3rd second
        if (!((samplesCaptured / halfPeriodSamples) % 2)) return ON;
        return -ON;
}

void Recorder::update_recordingDone() {
        System::sdCard.file_close();
        
        if (!System::dispatcher.performImmediateAction(
                Dispatcher::PATH_RECORDING_DONE)) return;
        
        status = Status::SENDING_FILE;
}

void Recorder::update_sendingFile() {
        if (System::httpClient.isBusy()) return;
        if (!System::dispatcher.performImmediateAction(
                Dispatcher::PATH_SEND_FILE)) return;
        
        status = Status::IDLE;
}

/******************************************************************************/

bool Recorder::startRecording() {
        if (status != Status::IDLE) 
        {
                Serial.println("Unable to start new recording. Recorder busy");
                return false;
        }
        
        Serial.println("Recording started");

        if (!System::sdCard.file_create(&Paths::recordings))
        {
                status = Status::IDLE;
                return false;
        }

        if (!Wav::writeHeader())
        {
                Serial.println("Failed to write WAV header");
                System::sdCard.file_close();
                status = Status::IDLE;
                return false;
        }
        
        started = System::now;
        status = Status::RECORDING;
        return true;
}
