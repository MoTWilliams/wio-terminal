#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Buffer {
        char *data;
        size_t capacity;
        size_t length;
} Buffer;

Buffer buffer_init(char *data, size_t capacity);
void buffer_clear(Buffer *b, bool clearAll);
bool buffer_copyFromCString(Buffer *d, const char *s);
bool buffer_appendCString(Buffer *b, const char *s);
bool buffer_appendChar(Buffer *b, const char c);

#endif