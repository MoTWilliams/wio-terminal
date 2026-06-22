#include <Arduino.h>
#include "buffer.h"
#include <stdbool.h>

/* data must not be NULL. capacity is data array size, including space for \0 */
Buffer buffer_init(char *data, size_t capacity) {
        Buffer b = { data, capacity, 0 };
        if (b.capacity > 0) b.data[0] = '\0';
        return b;
}

/* 
 * Clear buffer and reset length to 0. Use clearAll = false to only clear the 
 * populated bytes and clearAll = true to null out the whole buffer (typically
 * in error state). b must not be NULL
 */
void buffer_clear(Buffer *b, bool clearAll) {
        if (b->capacity == 0) return; // This should never happen
        size_t lengthToClear = clearAll ? b->capacity : b->length;

        for (size_t i = 0; i < lengthToClear && i < b->capacity; i++)
                b->data[i] = 0;

        if (b->capacity > 0) b->data[0] = '\0';
        b->length = 0;
}

/* Copy string contents into destination buffer */
bool buffer_copyFromCString(Buffer *d, const char *s) {
        int copiedBytes = snprintf(d->data, d->capacity, "%s", s);

        // Validate string format
        if (copiedBytes < 0)
        {
                buffer_clear(d, true);
                return false;
        }

        // Validate length
        if ((size_t)copiedBytes >= d->capacity)
        {
                buffer_clear(d, true);
                return false;
        }

        // Copy successful;
        d->length = copiedBytes;
        return true;
}

bool buffer_appendCString(Buffer *b, const char *s) {
        // Prevent underflow and validate append position
        if (b->length >= b->capacity) return false;

        // Perform append
        int copiedBytes = snprintf(
                b->data + b->length, b->capacity - b->length, "%s", s);
        
        // Validate string format
        if (copiedBytes < 0)
        {
                b->data[b->length] = '\0';
                return false;
        }

        // Validate new length
        size_t newLength = b->length + (size_t)copiedBytes;
        if (newLength >= b->capacity)
        {
                b->data[b->length] = '\0';
                return false;
        }

        // Append successful
        b->length = newLength;
        return true;
}

bool buffer_appendChar(Buffer *b, const char c) {
        // Don't append past the end of the buffer
        if (b->capacity == 0) return false;
        if (b->length >= b->capacity - 1) return false;

        // Perform append
        b->data[b->length] = c;
        b->length++;
        b->data[b->length] = '\0';
        return true;
}


