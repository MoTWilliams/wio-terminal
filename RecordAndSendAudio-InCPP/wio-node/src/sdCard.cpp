#include "sdCard.h"

namespace {
        constexpr char const* TEST_FILE = "test.txt";
}

SDCard::SDCard() {}

void SDCard::begin() {
        Serial.println("Initializing SD...");
        if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI))
        {
                // Halt if initialization fails
                Serial.println("SD initialization failed");
                while(true);
        }
        Serial.println("SD card ready");
}


bool SDCard::file_create() {
        file = SD.open(TEST_FILE, FILE_APPEND);

        if (!file) { Serial.println("Error opening test.txt"); return false; }

        status = Status::WRITING;

        return true;
} 


bool SDCard::file_append(const char* data) {
        if (status != Status::WRITING)
        {
                Serial.print("Error appending to file. ");
                Serial.println("SD not in WRITING mode");
                status = Status::IDLE;
                return false;
        }

        file.print(data);
        return true;
}

bool SDCard::file_append(const int data) {
        if (status != Status::WRITING)
        {
                Serial.print("Error appending to file. ");
                Serial.println("SD not in WRITING mode");
                status = Status::IDLE;
                return false;
        }
        
        file.print(data);
        return true;
}

bool SDCard::file_appendln(const char* data) {
        if (status != Status::WRITING)
        {
                Serial.print("Error appending to file. ");
                Serial.println("SD not in WRITING mode");
                status = Status::IDLE;
                return false;
        }
        
        file.println(data);
        return true;
}

bool SDCard::file_appendln(const int data) {
        if (status != Status::WRITING)
        {
                Serial.print("Error appending to file. ");
                Serial.println("SD not in WRITING mode");
                status = Status::IDLE;
                return false;
        }
        
        file.println(data);
        return true;
}

bool SDCard::file_appendln() {
        if (status != Status::WRITING)
        {
                Serial.print("Error appending to file. ");
                Serial.println("SD not in WRITING mode");
                status = Status::IDLE;
                return false;
        }
        
        file.println();
        return true;
}


void SDCard::file_finishWriting() {
        file.close();
}

void SDCard::file_printContents() {
        file = SD.open(TEST_FILE, FILE_READ);

        if (!file) { Serial.println("Error opening test.txt"); return; }
        
        status = Status::READING;

        Serial.printf("--- begin %s ---\n", file.name());
        while (file.available()) Serial.write(file.read());
        file.close();
        Serial.printf("--- begin %s ---\n", file.name());

        status = Status::IDLE;
}
