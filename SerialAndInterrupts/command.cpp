#include <Arduino.h>
#include "serial.h"
#include "sensor.h"
#include <stdbool.h>
#include <stdlib.h>

extern Sensor light;
extern Sensor humid;

static const char READ[] = "READ ";
static const char LIGHT[] = "LGHT ";
static const char HUMID[] = "HUMD ";

static bool busy = false;
static Sensor* sensor = NULL;
static int repeats = 0;
static unsigned long interval = 0;

static unsigned long lastRepStarted = 0;
static int completedReps = 0;

bool executing() { return busy; }

static bool parseCommand(char* str) {
  char* pIn = str;
  const char* pChk = READ;
  bool readParsed = false;
  char cmd = 0;

  // Move past "READ LGHT/HUMD"
  while (*pIn != '\0')
  {
    // "READ " parsed. Check for "LGHT " or "HUMD "
    if (*pChk == '\0')
    {
      if (!readParsed)
      {
        readParsed = true;
        cmd = *pIn;

        if (*pIn == 'L') pChk = LIGHT;
        else if (*pIn == 'H') pChk = HUMID;
        else return false;
      }
      // Leave the loop after "LGHT " or "HUMD "
      else break;
    }

    // Reject on a wrong character
    if (*pIn != *pChk) return false;
    pIn++;
    pChk++;
  }

  // Parse r--exactly one digit 0 < r < 6
  char* start = pIn;
  int r = strtol(pIn, &pIn, 10);
  if (pIn != start + 1) return false;  
  if (r < 1 || r > 5) return false;

  // Require exactly ", "
  if (*pIn != ',') return false;
  pIn++;
  if (*pIn != ' ') return false;
  pIn++;

  // Parse t--exactly one digit 0 < t < 6
  start = pIn;
  int t = strtol(pIn, &pIn, 10);
  if (pIn != start + 1) return false;
  if (t < 1 || t > 5) return false;
  
  // Reject trailing junk
  if (*pIn != '\0') return false;

  // Otherwise, parsing successful. Assign state, repeats and interval
  if (cmd == 'L') sensor = &light;
  else if (cmd == 'H') sensor = &humid;
  else return false;
  repeats = r;
  interval = t * 1000UL;

  return true;
}

void receiveAndStartSerialCommand() {
  if (busy) return;
  if (!Serial.available()) return;

  char buffer[128 + 1] = {0};
  serial_readline(buffer, sizeof(buffer));
  
  // Reject bad commands
  if (!parseCommand(buffer))
  {
    Serial.println("Invalid Command");
    return;
  }

  // First read happens immediately
  sensor->readSensor(sensor);
  completedReps = 1;

  if (repeats == 1)
  {
    Serial.println("Executed");
    return;
  }

  busy = true;
  lastRepStarted = millis();
}

void mindSerialCommand() {
  if (!busy) return;

  // Keep waiting if the interval has not yet passed
  if (millis() - lastRepStarted < interval) return;

  // Otherwise, perform the next rep and update state
  sensor->readSensor(sensor);
  lastRepStarted = millis();
  completedReps++;

  // Reps are finished. Reset and allow input again
  if (completedReps >= repeats) 
  {
    busy = false;
    completedReps = 0;
    Serial.println("Executed");
  }
}
