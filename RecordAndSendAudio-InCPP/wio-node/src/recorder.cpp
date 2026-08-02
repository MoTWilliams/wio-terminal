#include <Arduino.h>

#include "systemObjects.h"
#include "recorder.h"

namespace {
        // constexpr unsigned long RECORDING_DURATION = 3000;
        constexpr unsigned long SECOND = 1000;
        constexpr unsigned long MAX_SECONDS = 3;
}

Recorder::Recorder() {}

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
        unsigned long elapsed = now - started;
        static int count = 1;
        
        // "Recording" complete
        if (count > static_cast<int>(MAX_SECONDS))
        {
                Serial.printf("Recorded %d seconds", elapsed);
                Serial.println();

                count = 1;
                status = Status::FINISHING;
                return;
        }

        if (elapsed < SECOND) return;

        if (!System::sdCard.file_append("test ") ||
            !System::sdCard.file_appendln((const int)count))
        {
                Serial.println("File append failed");
                System::sdCard.file_finishWriting();
                status = Status::IDLE;
                return;
        }

        count++;
        started = now;
}

void Recorder::update_finishing(unsigned long now) {
        if (!System::httpClient.POST(Dispatcher::PATH_RECORDING_DONE))
                // Reset and return
                { status = Status::IDLE; return; }
        
        System::sdCard.file_finishWriting();
        System::sdCard.file_printContents();
        
        status = Status::IDLE;
}

/******************************************************************************/

void Recorder::startRecording(unsigned long now) {
        if (status != Status::IDLE) return;
        
        Serial.println("Recording started");
        
        if (!System::sdCard.file_create()) { status = Status::IDLE; return; }
        started = now;

        status = Status::RECORDING;
}
