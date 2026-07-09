#include <Arduino.h>
#include "httpClient.h"
#include "recorder.h"

namespace {
        constexpr unsigned long RECORDING_DURATION = 3000;
}

Recorder::Recorder(HTTPClient &client) : httpClient(client) {}

void Recorder::begin() {}

/******************************************************************************/

void Recorder::update(unsigned long now) {
        switch (status)
        {
                case Status::IDLE: return;
                case Status::RECORDING: update_recording(now); return;
                case Status::FINISHING: update_finishing(now); return;
        }
}

void Recorder::update_recording(unsigned long now) {
        unsigned long elapsed = now - startedRecording;
        
        // Record for 3 seconds
        if (elapsed < RECORDING_DURATION) return;

        Serial.print("Recording duration elapsed: ");
        Serial.println(elapsed);

        if (!httpClient.POST("/recording/stopped")) return;
        
        status = Status::FINISHING;
}

void Recorder::update_finishing(unsigned long now) {
        status = Status::IDLE;
}

/******************************************************************************/

void Recorder::startRecording(unsigned long now) {
        if (status != Status::IDLE) return;
        
        if (!httpClient.POST("/recording/started")) return;
        status = Status::RECORDING;

        // Might be able to change this to now
        startedRecording = now;
}
