#ifndef LOGGING_H
#define LOGGING_H

void log_info(char* buffer, size_t size);
void log_debug(char* buffer, size_t size);
void log_warn(char* buffer, size_t size);
void log_err(char* buffer, size_t size);
void log_crit(char* buffer, size_t size);

#endif