#ifndef COMM_H
#define COMM_H

void wifiStation_init(void);

void server_init(void);
void server_update(void);

void client_POST(const char *path, const char *body);
void client_GET(const char *path);
void client_update(void);

#endif