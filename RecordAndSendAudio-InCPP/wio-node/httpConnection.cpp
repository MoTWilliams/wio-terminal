#include <Arduino.h>
#include <rpcWiFi.h>
#include <WiFiClient.h>
#include "wifiStation.h"
#include "connectionMonitor.h"
#include "secrets.h"
#include "lineBuffer.h"
#include "httpConnection.h"

namespace {
        constexpr unsigned long BYTES_TO_READ_TIMEOUT = 3000;
        
        // Received requests
        constexpr char* PATH_RECORD = "/record/start";
        constexpr char* PATH_STATUS = "/status";

        constexpr char* ACTION_RECORD = "Record 3 seconds";
        constexpr char* ACTION_STATUS = "Report system status";
        
        // Sent requests
        constexpr char* PATH_RECORDING_STARTED = "/recording/started";
        constexpr char* PATH_RECORDING_STOPPED = "/recording/stopped";
        
        constexpr char* ACTION_RECORDING_STARTED = "Recording started";
        constexpr char* ACTION_RECORDING_STOPPED = "Recording stopped";
        
        // Error or whatever
        constexpr char* ACTION_NOT_HANDLED = "Action not handled";
}

HTTPConnection::HTTPConnection(
        ConnectionMonitor &monitor, WiFiClient &client, LineBuffer &buffer)
        : connectionMonitor(monitor), client(client), lineBuffer(buffer) {}

void HTTPConnection::reset() {
        client.stop();

        lineBuffer.clear();

        method_ = Method::UNKNOWN;
        action_ = Action::NONE;
}

void HTTPConnection::updateLastProgress() { lastProgress = millis(); }

void HTTPConnection::setMethod(const char* method) {
        if (!strcmp(method, "GET")) method_ = Method::GET;
        else if (!strcmp(method, "POST")) method_ = Method::POST;
        else method_ = Method::UNKNOWN;
}

void HTTPConnection::setAction(const char* path) {
        if (!strcmp(path, PATH_RECORD)) action_ = Action::RECORD;
        else if (!strcmp(path, PATH_STATUS)) action_ = Action::STATUS;
        else if (!strcmp(path, PATH_RECORDING_STARTED))
                action_ = Action::RECORDING_STARTED;
        else if (!strcmp(path, PATH_RECORDING_STOPPED))
                action_ = Action::RECORDING_STOPPED;
        else action_ = Action::NONE;
}

const char* HTTPConnection::method() const {
        switch (method_)
        {
                case Method::GET: return "GET";
                case Method::POST: return "POST";
                default: return "UNKNOWN";
        }
}

const char* HTTPConnection::action() const {
        switch (action_)
        {
                case Action::RECORD: return ACTION_RECORD;
                case Action::STATUS: return ACTION_STATUS;
                case Action::RECORDING_STARTED: return ACTION_RECORDING_STARTED;
                case Action::RECORDING_STOPPED: return ACTION_RECORDING_STOPPED;
                default: return ACTION_NOT_HANDLED;
        }
}

bool HTTPConnection::readChar(char &c) {
        if (!connectionMonitor.checkConnection(client, lastProgress))
                { reset(); return false; }
        
        unsigned long now = millis();

        // Check continuously for bytes to read. Give up and reset after 3 sec
        if (!client.available())
        {
                if (now - lastProgress >= BYTES_TO_READ_TIMEOUT) reset();
                return false;
        }

        // Byte available. Attempt read
        int cVal = client.read();
        if (cVal < 0)                           // Byte read failed
        {
                reset();
                connectionMonitor.handleConnectionLost();
                return false;
        }

        // Byte read successful
        lastProgress = now;
        c = static_cast<char>(cVal);
        return true;
}

bool HTTPConnection::processChar(char c) {
        if (c == '\r') return false;
        if (c == '\n') return true;

        lineBuffer.appendChar(c);
        return false;
}

bool HTTPConnection::recordingRequested() { return action_ == Action::RECORD; }
