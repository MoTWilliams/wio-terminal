#ifndef WIFI_STATION_H
#define WIFI_STATION_H

class WiFiStation {
public:
        WiFiStation();
        
        void begin();
        void update();
        
        void onDisconnected();
        bool isOffline();
private:
        enum class Status { OFFLINE, SEARCHING, ONLINE };

        Status status = Status::OFFLINE;

        unsigned long startedChecking = 0;
        unsigned long lastChecked = 0;
        unsigned long waitInterval;
        unsigned long startedWaiting = 0;
        int tries = 0;

        void update_offline();
        void update_searching();
        void update_online();
};

#endif