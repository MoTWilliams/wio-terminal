#ifndef SYSTEM_OBJECTS_H
#define SYSTEM_OBJECTS_H

#include "wifiStation.h"
#include "connectionMonitor.h"
#include "actionDispatcher.h"
#include "httpServer.h"
#include "httpClient.h"
#include "sdCard.h"
#include "recorder.h"

namespace System {
        extern WiFiStation wifiStation;
        extern ConnectionMonitor connectionMonitor;
        extern Dispatcher dispatcher;
        extern HTTPServer httpServer;
        extern HTTPClient httpClient;
        extern SDCard sdCard;
        extern Recorder recorder;
}

#endif