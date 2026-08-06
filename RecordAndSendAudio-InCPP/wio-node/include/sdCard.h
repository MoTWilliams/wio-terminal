#ifndef SD_CARD_H
#define SD_CARD_H

#include <Arduino.h>
#include <SPI.h>
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"
#include "paths.h"

class SDCard {
public:
        SDCard();

        void begin();

        void file_selectSet(const FileSet* fileSet);
        bool file_create(const FileSet* fileSet);

        bool file_append(const char* data);
        bool file_append(const int data);
        bool file_appendln(const char* data);
        bool file_appendln(const int data);
        bool file_appendln();
        
        bool file_openToRead();

        bool file_printContents();
        size_t file_size();

        // This will be much larger for audio files. I want to force the tiny
        // text file to send in chunks and have leftover bytes
        static const size_t FILE_CHUNK_SIZE = 5;
        size_t file_readChunk(uint8_t* buffer);
        
        bool file_EOF();

        void file_close();
private:
        enum class Status { IDLE, WRITING, READING, FINISHING };

        Status status = Status::IDLE;
        File file;
        const FileSet* set = nullptr;
        size_t bytesRead = 0;
        
        bool file_rename(const char* oldName, const char* newName);
        bool file_delete(const char* fileName);

        bool file_rotate();
};

#endif