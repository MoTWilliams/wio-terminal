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

void Recorder::update() {
        switch (status)
        {
                case Status::IDLE: return;
                case Status::RECORDING: update_recording(); return;
                case Status::RECORDING_DONE: update_recordingDone(); return;
                case Status::SENDING_FILE: update_sendingFile(); return;
        }
}

void Recorder::update_recording() {
        unsigned long elapsed = System::now - started;
        static int count = 1;
        
        // "Recording" complete
        if (count > static_cast<int>(MAX_SECONDS))
        {
                Serial.printf("Recorded %d seconds", elapsed);
                Serial.println();

                // TODO: Then set pending dispatcher action to send the file

                count = 1;
                status = Status::RECORDING_DONE;
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
        started = System::now;
}

void Recorder::update_recordingDone() {
        System::sdCard.file_finishWriting();
        System::sdCard.file_printContents();
        
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
        
        if (!System::sdCard.file_create())
        {
                status = Status::IDLE;
                return false;
        }
        
        started = System::now;
        status = Status::RECORDING;
        return true;
}
