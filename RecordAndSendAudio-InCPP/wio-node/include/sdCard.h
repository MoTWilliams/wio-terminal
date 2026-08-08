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

        size_t file_append(const char* data);
        size_t file_append(const int data);
        size_t file_appendln(const char* data);
        size_t file_appendln(const int data);
        size_t file_appendln();
        size_t file_write(const uint8_t* data, size_t length);
        size_t file_write(const int16_t* data, size_t numSamples);
        size_t file_write(const uint8_t data);
        size_t file_write(const uint16_t data);
        size_t file_write(const uint32_t data);
        
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
        
        bool beginAppend();

        bool file_rename(const char* oldName, const char* newName);
        bool file_delete(const char* fileName);

        bool file_rotate();
};

#endif