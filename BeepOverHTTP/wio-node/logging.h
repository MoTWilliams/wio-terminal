#ifndef LOGGING_H
#define LOGGING_H

void log_init(void);

void build_log(const char* msg, ...);

// Use similar syntax to printf()
void log_info(char* msg, ...);
void log_debug(char* msg, ...);
void log_warn(char* msg, ...);
void log_err(char* msg, ...);
void log_crit(char* msg, ...);

#endif