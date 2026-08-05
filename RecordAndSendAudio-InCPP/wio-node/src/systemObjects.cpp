#include "systemObjects.h"

namespace System {
        unsigned long now = 0;
        
        WiFiStation wifiStation;
        ConnectionMonitor connectionMonitor;
        Dispatcher dispatcher;
        HTTPServer httpServer;
        HTTPClient httpClient;
        SDCard sdCard;
        Recorder recorder;
}