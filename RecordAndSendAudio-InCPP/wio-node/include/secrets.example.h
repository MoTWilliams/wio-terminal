#ifndef SECRETS_H
#define SECRETS_H

#include <Arduino.h>

struct NetworkConfig {
        const char* ssid_;
        const char* password_;
        const char* host_;
        uint16_t port_;
};

class Secrets {
public:
        static const char* ssid() { return active.ssid_; }
        static const char* password() { return active.password_; }
        static const char* host() { return active.host_; }
        static uint16_t port() { return active.port_; }
private:
        static constexpr NetworkConfig home {
                "Home ssd", "Home password",
                "192.168.42.69", 8080
        };

        static constexpr NetworkConfig location2 {
                "Location 2 ssd", "Location 2 password",
                "192.168.69.42", 8080
        };

        static constexpr const NetworkConfig& active = home;
};

#endif