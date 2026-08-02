#ifndef CONNECTION_MONITOR_H
#define CONNECTION_MONITOR_H

#include <WiFiClient.h>

class ConnectionMonitor {
public:
        ConnectionMonitor();

        /* Reset connection if this fails */
        bool checkConnection(WiFiClient &client);

        /* Reset connection if this fails */
        bool checkConnection(WiFiClient &client, unsigned long lastProgress);

        /* Reset connection before calling this */
        void handleConnectionLost();
};

#endif