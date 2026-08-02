#include <Arduino.h>
#include "systemObjects.h"
#include "actionDispatcher.h"

namespace {}

/**************************************************************************** */

void Dispatcher::update(unsigned long now) {
        if (status != Status::DISPATCHING) return;
        
        if (pendingAction_ == Action::NONE) return;
        
        switch (pendingAction_)
        {
                case Action::START_RECORDING:
                        System::recorder.startRecording(now); break;
                case Action::NOTIFY_STOPPED:
                        System::httpClient.POST(PATH_RECORDING_DONE); break;
                case Action::SEND_FILE:
                        // We'll figure  out how to attach the file eventually
                        System::httpClient.POST(PATH_SEND_FILE); break;
                default:
                        Serial.println("Action not supported"); break;
        }

        pendingAction_ = Action::NONE;
        status = Status::IDLE;
}

/**************************************************************************** */

bool Dispatcher::setPendingAction(const char* path) {
        if (pendingAction_ != Action::NONE) return false;

        if (!strcmp(path, PATH_START_RECORDING))
                pendingAction_ = Action::START_RECORDING;
        else if (!strcmp(path, PATH_RECORDING_DONE))
                pendingAction_ = Action::NOTIFY_STOPPED;
        else if (!strcmp(path, PATH_SEND_FILE))
                pendingAction_ = Action::SEND_FILE;
        else pendingAction_ = Action::UNKNOWN;

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