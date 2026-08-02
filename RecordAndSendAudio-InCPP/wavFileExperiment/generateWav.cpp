#include <cstdint>
#include <fstream>
using namespace std;

namespace littleEndian {
        void writeToFile(ostream& out, int16_t value) {
                uint16_t raw = static_cast<uint16_t>(value);
                
                out.put(raw & 0xFF);
                out.put((raw >> 8) & 0xFF);
        }
        
        void writeToFile(ostream& out, uint16_t value) {
                out.put(value & 0xFF);
                out.put((value >> 8) & 0xFF);
        }

        void writeToFile(ostream& out, uint32_t value) {
                out.put(value & 0xFF);
                out.put((value >> 8) & 0xFF);
                out.put((value >> 16) & 0xFF);
                out.put((value >> 24) & 0xFF);
        }
}
using namespace littleEndian;

int main() {
        ofstream f( "test.wav", ios::binary );
        if (!f) return 1;

        const uint32_t sampleRate = 1600; // 5
        const uint32_t durationSec = 3;
        const uint16_t numChannels = 1; // 4
        const uint16_t bitsPerSample = 16; // 8
        
        const uint16_t audioFmt = 1; // (PCM) 3
        const uint32_t fmtInfoChunkSize = 16; // 2

        const uint32_t numSamples = sampleRate * durationSec;
        const uint16_t bytesPerSample = bitsPerSample / 8;
        const uint16_t blockAlign = numChannels * bytesPerSample; // 7
        const uint32_t byteRate = sampleRate * blockAlign; // 6
        const uint32_t dataSize = numSamples * blockAlign; //9
        const uint32_t chunkSize = 36 + dataSize; // 1
        
        // Write WAV header
        f.write("RIFF", 4); writeToFile(f, chunkSize); f.write("WAVE", 4);

        f.write("fmt ", 4); 
        writeToFile(f, fmtInfoChunkSize);
        writeToFile(f, audioFmt);
        writeToFile(f, numChannels);
        writeToFile(f, sampleRate);
        writeToFile(f, byteRate);
        writeToFile(f, blockAlign);
        writeToFile(f, bitsPerSample);

        f.write("data", 4); writeToFile(f, dataSize);
        
        // Generate a series of beeps
        const uint16_t ON = 12000;
        const uint16_t OFF = 0;

        const uint32_t halfPeriodSamples = sampleRate / (440 * 2);
        const uint32_t oneSecSamples = numSamples / 3;

        for (uint32_t i = 0; i < oneSecSamples; i++)
        {
                int16_t sample;
                if ((i / halfPeriodSamples) % 2 == 0) sample = ON;
                else sample = -ON;

                writeToFile(f, sample);
        }

        for (uint32_t i = oneSecSamples; i < 2 * oneSecSamples; i++)
        {
                uint16_t sample = 0;
                writeToFile(f, sample);
        }
        
        for (uint32_t i = 2 * oneSecSamples; i < numSamples; i++)
        {
                int16_t sample;
                if ((i / halfPeriodSamples) % 2 == 0) sample = ON;
                else sample = -ON;

                writeToFile(f, sample);
        }

        f.close();

        return 0;
}