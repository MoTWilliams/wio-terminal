#ifndef SD_CARD_H
#define SD_CARD_H

#include <Arduino.h>
#include <SPI.h>
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"

class SDCard {
public:
        SDCard();

        void begin();

        bool file_create();
        
        bool file_append(const char* data);
        bool file_append(const int data);
        bool file_appendln(const char* data);
        bool file_appendln(const int data);
        bool file_appendln();
        
        void file_finishWriting();

        void file_printContents();

        
private:
        enum class Status { IDLE, WRITING, READING, FINISHING };

        Status status = Status::IDLE;
        File file;

        bool file_rename(const char* oldName, const char* newName);
        bool file_delete(const char* fileName);

        bool file_rotate();
};

#endif