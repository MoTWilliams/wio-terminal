#ifndef LINE_BUFFER_H
#define LINE_BUFFER_H

class ConnectionMonitor;

class LineBuffer {
public:
        void clear();
        bool appendChar(const char c);
        bool append(const char* text);

        char* data();
        const char* c_str() const;
        size_t length() const;
        bool didOverflow() const;
private:
        static constexpr size_t BUFFER_SIZE = 512 + 1;
        char buffer[BUFFER_SIZE] = {0};
        char *p = buffer;
        size_t length_ = 0;

        bool overflow = false;
};

#endif