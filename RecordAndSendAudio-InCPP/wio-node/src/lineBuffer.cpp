#include <Arduino.h>
#include "connectionMonitor.h"
#include "lineBuffer.h"

namespace {}

void LineBuffer::clear() {
        buffer[0] = '\0';
        p = buffer;
        length_ = 0;
        overflow = false;
}

bool LineBuffer::appendChar(const char c) {
        if (length_ >= BUFFER_SIZE - 1) // Buffer overflow
        {
                overflow = true;
                return false;
        }

        *p++ = c;
        *p = '\0';
        length_++;
        return true;
}

bool LineBuffer::append(const char* text) {
        while (*text != '\0')
                if (!appendChar(*text++)) return false;
        
        return true;
}

char* LineBuffer::data() { return buffer; }

const char* LineBuffer::c_str() const { return buffer; }

size_t LineBuffer::length() const{ return length_; }

bool LineBuffer::didOverflow() const { return overflow; }