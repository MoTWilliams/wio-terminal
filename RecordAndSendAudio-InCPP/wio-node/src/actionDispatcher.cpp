#include <Arduino.h>
#include "systemObjects.h"
#include "actionDispatcher.h"

namespace {}

/******************************************************************************/

bool Dispatcher::selectAction(const char* path) {
        if (pendingAction_ != Action::NONE) 
        {
                Serial.println("Action not NONE. Cannot select action");
                return false;
        }

        if (!strcmp(path, PATH_START_RECORDING))
                pendingAction_ = Action::START_RECORDING;
        else if (!strcmp(path, PATH_RECORDING_DONE))
                pendingAction_ = Action::NOTIFY_STOPPED;
        else if (!strcmp(path, PATH_SEND_FILE))
                pendingAction_ = Action::SEND_FILE;
        else pendingAction_ = Action::UNKNOWN;

        return true;
}

bool Dispatcher::performAction() {
        if (pendingAction_ == Action::NONE) return false;

        switch (pendingAction_)
        {
                case Action::START_RECORDING:
                        return System::recorder.startRecording();
                case Action::NOTIFY_STOPPED:
                        return System::httpClient.POST(PATH_RECORDING_DONE);
                case Action::SEND_FILE:
                        // We'll figure  out how to attach the file eventually
                        return System::httpClient.POST(PATH_SEND_FILE);
                default:
                        Serial.println("Action not supported"); return false;
        }
}

/**************************************************************************** */

void Dispatcher::update() {
        if (status != Status::DISPATCHING) return;
        
        if (!performAction())
        {
                Serial.println("Dispatcher action failed");
        }

        pendingAction_ = Action::NONE;
        status = Status::IDLE;
}

/**************************************************************************** */

bool Dispatcher::performImmediateAction(const char* path) {
        if (status != Status::IDLE)
        {
                Serial.println("No action performed. Dispatcher busy");
                return false;
        }

        if (!selectAction(path)) return false;
        if (!performAction()) return false;
        
        pendingAction_ = Action::NONE;
        return true;
}

bool Dispatcher::setPendingAction(const char* path) {
        if (status != Status::IDLE)
        {
                Serial.println("No pending action selected. Dispatcher busy");
                return false;
        }

        if (!selectAction(path)) return false;
        
        status = Status::PENDING;
        return true;
}

bool Dispatcher::performPendingAction() {
        if (status != Status::PENDING) return false;

        status = Status::DISPATCHING;
        return true;
}

const char* Dispatcher::pendingAction() {
        switch(pendingAction_)
        {
                case Action::NONE: return LOG_NO_ACTION;
                case Action::START_RECORDING:
                        return LOG_START_RECORDING;
                case Action::NOTIFY_STOPPED:
                        return LOG_RECORDING_DONE;
                case Action::SEND_FILE:
                        return LOG_SEND_FILE;
                default: return LOG_UNKNOWN_ACTION;
        }
}