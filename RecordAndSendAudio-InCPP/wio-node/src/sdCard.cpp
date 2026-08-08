#include "sdCard.h"

SDCard::SDCard() {}

void SDCard::begin() {
        Serial.println("Initializing SD...");
        if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI))
        {
                // Halt if initialization fails
                Serial.println("SD initialization failed");
                while(true);
        }
        

        if (!SD.exists("/recordings")) {
                if (!SD.mkdir("/recordings")) {
                        Serial.println("Failed to create /recordings");
                        while (true);
                }

                Serial.println("Created /recordings directory");
        }
        Serial.println("/recordings directory found");

        if (!SD.exists("/logs")) {
                if (!SD.mkdir("/logs")) {
                        Serial.println("Failed to create /logs directory");
                        while (true);
                }

                Serial.println("Created /logs directory");
        }
        Serial.println("/logs directory found");

        Serial.println("SD card ready");
}


bool buildPath(char* buffer, const char* dir, const char* name) {
        buffer[0] = '\0';
        
        size_t dirLen = strlen(dir);
        size_t nameLen = strlen(name);
        
        size_t pathLen = dirLen + nameLen;
        if (pathLen > Paths::MAX_PATH_LEN)
        {
                Serial.println("SDCard: path too long");
                return false;
        }

        size_t written = snprintf(
                buffer, Paths::PATH_BUFFER_SIZE, "%s", dir);
        if (written != dirLen) { buffer[0] = '\0'; return false; }

        written = snprintf(
                buffer + dirLen,
                Paths::PATH_BUFFER_SIZE - dirLen, 
                "%s", name
        );
        if (written != nameLen) { buffer[0] = '\0'; return false; }

        return true;
}

// Probably need this for sending the log file
void SDCard::file_selectSet(const FileSet* fileSet) {
        set = fileSet;
}

/****************************************************************************** 
 *                                   WRITE                                    *
 ******************************************************************************/

bool SDCard::file_create(const FileSet* fileSet) {
        set = fileSet;
        
        if (!file_rotate())
        {
                Serial.printf("File rotation failed");
                return false;
        }
        
        char current_[Paths::PATH_BUFFER_SIZE] = {0};
        if (!buildPath(current_, set->dir, set->current)) return false;
        
        file = SD.open(current_, FILE_WRITE);

        if (!file) 
        {
                Serial.printf("Error opening %s\n", current_);
                return false;
        }

        status = Status::WRITING;
        return true;
}


bool SDCard::beginAppend() {
        if (status != Status::WRITING)
        {
                Serial.print("Error appending to file. ");
                Serial.println("SD not in WRITING mode");
                status = Status::IDLE;
                return false;
        }
        return true;
}

size_t SDCard::file_append(const char* data) {
        if (!beginAppend()) return 0;
        return file.print(data);
}

size_t SDCard::file_append(const int data) {
        if (!beginAppend()) return 0;
        return file.print(data);
}

size_t SDCard::file_appendln(const char* data) {
        if (!beginAppend()) return 0;
        return file.println(data);
}

size_t SDCard::file_appendln(const int data) {
        if (!beginAppend()) return 0;
        return file.println(data);
}

size_t SDCard::file_appendln() {
        if (!beginAppend()) return 0;
        return file.println();
}

size_t SDCard::file_write(const uint8_t* data, size_t length) {
        if (!beginAppend()) return 0;
        return file.write(data, length);
}

size_t SDCard::file_write(const int16_t* data, size_t numSamples) {
        return file_write(
                reinterpret_cast<const uint8_t*>(data),
                numSamples * sizeof(int16_t)
        );
}

size_t SDCard::file_write(const uint8_t data) {
        return file_write(&data, 1);
}

size_t SDCard::file_write(const uint16_t data) {
        return file_write(
                reinterpret_cast<const uint8_t*>(&data), sizeof(data)
        );
}

size_t SDCard::file_write(const uint32_t data) {
        return file_write(
                reinterpret_cast<const uint8_t*>(&data), sizeof(data)
        );
}

/****************************************************************************** 
 *                                    READ                                    *
 ******************************************************************************/

// Opens "current" of the active FileSet (recording or log). May eventually
// (or in another iteration) generalize this to select current or backup
bool SDCard::file_openToRead() {
        char current_[Paths::PATH_BUFFER_SIZE] = {0};
        if (!buildPath(current_, set->dir, set->current)) return false;

        file = SD.open(current_, FILE_READ);

        if (!file) 
        {
                Serial.printf("Error opening %s\n", current_);
                return false;
        }

        status = Status::READING;
        return true;
}

// This only prints "current" of the active FileSet. It can eventually be 
// refactored to retrieve a file by name, but I'll do that later
bool SDCard::file_printContents() {
        if (status != Status::READING)
        {
                Serial.printf("Error reading from %s. ", set->current);
                Serial.println("SD not in READING mode");
                status = Status::IDLE;
                return false;
        }

        Serial.printf("--- begin %s ---\n", file.name());
        while (file.available()) Serial.write(file.read());
        file.close();
        Serial.printf("--- end %s ---\n", file.name());
        return true;
}

size_t SDCard::file_size() {
        if (status == Status::READING) return file.size();
        
        if (status == Status::WRITING)
        {
                file.flush();
                return file.size();
        }

        Serial.printf("No file open. Could not check size");
        return 0;
}

size_t SDCard::file_readChunk(uint8_t* buffer) {
        if (status != Status::READING)
        {
                Serial.printf("Error reading chunk from %s. ", set->current);
                Serial.println("SD not in READING mode");
                status = Status::IDLE;
                return 0;
        }

        size_t newBytesRead = file.read(buffer, FILE_CHUNK_SIZE);
        if (newBytesRead == 0)
        {
                Serial.println("Read chunk failed");
                file_close();
                return 0;
        }
        bytesRead += newBytesRead;

        return newBytesRead;
}

bool SDCard::file_EOF() {
        if (status != Status::READING)
        {
                Serial.println("EOF check failed. No file open to read");
                return false;
        }

        return bytesRead >= file.size();
}

/******************************************************************************/

void SDCard::file_close() {
        file.close();
        bytesRead = 0;
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
        char current_[Paths::PATH_BUFFER_SIZE] = {0};
        char backup_[Paths::PATH_BUFFER_SIZE] = {0};

        if (!buildPath(current_, set->dir, set->current)) return false;
        if (!buildPath(backup_, set->dir, set->backup)) return false;
        
        if (!file_delete(backup_))
        {
                Serial.println("Failed to delete old backup");
                return false;
        }

        if (!file_rename(current_, backup_))
        {
                Serial.println("Failed to rename existing file");
                return false;
        }

        Serial.printf("Renamed %s to %s\n", current_, backup_);
        return true;
}
