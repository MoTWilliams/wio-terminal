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
        // void update(unsigned long now);

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

        // void update_writing(unsigned long now);
        // void update_reading(unsigned long now);
        // void update_finishing(unsigned long now);
};

#endif