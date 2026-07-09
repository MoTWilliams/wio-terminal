#ifndef CONNECTION_MONITOR_H
#define CONNECTION_MONITOR_H

#include <WiFiClient.h>
class WiFiStation;

class ConnectionMonitor {
public:
        ConnectionMonitor(WiFiStation &station);

        /* Reset connection if this fails */
        bool checkConnection(WiFiClient &client);

        /* Reset connection if this fails */
        bool checkConnection(WiFiClient &client, unsigned long lastProgress);

        /* Reset connection before calling this */
        void handleConnectionLost();
private:
        WiFiStation &wifiStation;
};

#endif