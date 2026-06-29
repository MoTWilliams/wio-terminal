#include <Arduino.h>
#include "recorder.h"
#include <stdbool.h>
#include "network.h"

typedef enum {
        IDLE,
        RECORDING,
        FINISHING
} RecorderStatus;

static RecorderStatus status = IDLE;

static const unsigned long RECORDING_DURATION = 3000;

static unsigned long startedRecording;

void recorder_init(void) {

}

void recorder_update(void) {
        if (wifiStation_isOffline()) return;
        if (status == IDLE) return;

        unsigned long now = millis();

        if (status == RECORDING)
        {
                // Record for 3 seconds
                if (now - startedRecording < RECORDING_DURATION) return;

                Serial.println("Recording stopped. Processing...");
                client_POST("/recording/stopped");

                status = FINISHING;
                return;
        }

        if (status == FINISHING)
        {
                Serial.println("Finished processing recording");

                status = IDLE;
                return;
        }
}

void recorder_startRecording(void) {
        if (wifiStation_isOffline()) return;
        if (status != IDLE)
        {
                Serial.println("START ABORTED: Recorder busy");
                return;
        }
        status = RECORDING;
        startedRecording = millis();
}
