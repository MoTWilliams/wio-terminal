#ifndef NETWORK_H
#define NETWORK_H

#include <WiFiClient.h>

void wifiStation_init(void);
void wifiStation_update(void);

void wifiStation_onDisconnected(void);
bool wifiStation_isOffline(void);

/* Call reset*() if this fails. Call with lastProgress = NULL if timeout doesn't
 matter
 */
bool wifiStation_checkClientConnection(
        WiFiClient client, unsigned long *lastProgress);
/* Call reset*() before calling this */
void wifiStation_handleClientConnectionLost(WiFiClient client);

void server_init(void);
void server_update(void);

void client_POST(const char *path); // Only empty POST requests
void client_update(void);

#endif