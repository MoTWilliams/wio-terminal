#include "sdCard.h"

namespace {
        constexpr char const* TEST_FILE = "test.txt";
        constexpr char const* BACKUP_FILE = "~test.txt";
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
        if (!file_rotate())
        {
                Serial.printf("File rotation failed");
                return false;
        }
        
        file = SD.open(TEST_FILE, FILE_WRITE);

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
        status = Status::IDLE;
}

void SDCard::file_printContents() {
        if (status != Status::IDLE)
        {
                Serial.println("Print file failed. SD card busy");
                return;
        }
        
        file = SD.open(TEST_FILE, FILE_READ);

        if (!file) { Serial.println("Error opening test.txt"); return; }
        
        status = Status::READING;

        Serial.printf("--- begin %s ---\n", file.name());
        while (file.available()) Serial.write(file.read());
        file.close();
        Serial.printf("--- end %s ---\n", file.name());

        status = Status::IDLE;
}

bool SDCard::file_rename(const char* oldName, const char* newName) {
        // Backup file won't exist on the first rotation. That's okay
        if (!SD.exists(oldName)) return true;

        if (SD.rename(oldName, newName)) return true;

        // Rename failed
        Serial.printf("Failed to rename %s to %s", oldName, newName);
        return false;
}

bool SDCard::file_delete(const char* fileName) {
        // File won't exist on the first rotation. That's okay
        if (!SD.exists(fileName)) return true;

        if (SD.remove(fileName)) return true;

        // Remove failed
        Serial.printf("Failed to delete %s\n", fileName);
        return false;
}

bool SDCard::file_rotate() {
        if (!file_delete(BACKUP_FILE))
        {
                Serial.println("Failed to delete old backup");
                return false;
        }

        if (!file_rename(TEST_FILE, BACKUP_FILE))
        {
                Serial.println("Failed to rename existing file");
                return false;
        }

        Serial.printf("Renamed %s to %s\n", TEST_FILE, BACKUP_FILE);
        return true;
}