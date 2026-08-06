#ifndef PATHS_H
#define PATHS_H

#include <Arduino.h>

struct FileSet {
        const char* dir;
        const char* current;
        const char* backup;
};

struct Paths {
        static const size_t MAX_PATH_LEN = 256;
        static const size_t PATH_BUFFER_SIZE = MAX_PATH_LEN + 1; 
        
        static const size_t MAX_ENDPOINT_LEN = 64;
        static const size_t ENDPOINT_BUFFER_SIZE = MAX_ENDPOINT_LEN + 1;

        static constexpr char const* ENDPOINT_RECORDING_START = 
                "/recorder/start";
        static constexpr char const* ENDPOINT_RECORDING_DONE = "/recorder/done";
        static constexpr char const* ENDPOINT_FILE_SEND = "/file/send";

        // Eventually, there will also be a FileSet for logs
        static const FileSet recordings;
};

#endif