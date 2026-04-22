/*
  Morgan Williams
  CSCE 3612.001
  Lab 4: Wio Serial and Interrupts - command.cpp
  Apr 22, 2026
*/
#include <Arduino.h>
#include "serial.h"
#include "sensor.h"
#include "button.h"
#include <stdbool.h>
#include <stdlib.h>

/******************************************************************************
 *                            BUTTON COMMAND STATE                            * 
 ******************************************************************************/
extern Button accel_b;
extern Button temp_b;

// Identifies which button triggered an interrupt request. SET BY button ISRs.
// SHARED WITH main loop.
extern volatile Button* triggeredButton;

// Internal ISR flag indicating presence of a pending sensor request
static volatile bool sensorRequested;

/******************************************************************************
 *                            SERIAL COMMAND STATE                            * 
 ******************************************************************************/
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

/******************************************************************************
 *                     INTERRUPT SERVICE ROUTINES (ISRs)                      * 
 ******************************************************************************/
void requestAccel() {
  sensorRequested = true;
  triggeredButton = &accel_b;
}

void requestTemp() {
  sensorRequested = true;
  triggeredButton = &temp_b;
}

/******************************************************************************
 *                          BUTTON COMMAND HANDLING                           * 
 ******************************************************************************/
void buttonCommand_execute(Button* b) {
  if (!sensorRequested) return;
  if (triggeredButton != b) return;

  sensorRequested = false;

  if (button_pressed(b)) {
    if (busy)
    {
      Serial.println("Timed operation in progress");
      return;
    }
    b->sensor->readSensor(b->sensor);
  }
}

/******************************************************************************
 *                  SERIAL COMMAND EXECUTION & CONTINUATION                   * 
 ******************************************************************************/
static bool serialCommand_parse(char* str) {
  char* pIn = str;
  const char* pChk = READ;
  bool readParsed = false;
  char cmd = 0;

  // Validate "READ " followed by either "LGHT " or "HUMD "
  while (*pIn != '\0')
  {
    // "READ " parsed; Check for sensor code
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
      // Leave the loop after parsing sensor code
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
  
  // Reject on any trailing characters
  if (*pIn != '\0') return false;

  // Otherwise, parsing successful. Assign timed-operation state
  if (cmd == 'L') sensor = &light;
  else if (cmd == 'H') sensor = &humid;
  else return false;
  repeats = r;
  interval = t * 1000UL;  // ms

  return true;
}

void serialCommand_execute() {
  if (!Serial.available()) return;
  
  // Reject new serial commands while timed operation is active
  if (busy)
  {
    while (Serial.available()) Serial.read();
    Serial.println("Timed operation in progress");
    return;
  }

  char buffer[128 + 1] = {0};
  serial_readline(buffer, sizeof(buffer));
  
  // Reject invalid commands
  if (!serialCommand_parse(buffer))
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

void serialCommand_update() {
  if (!busy) return;

  // Wait until next interval elapses
  if (millis() - lastRepStarted < interval) return;

  // Otherwise, perform the next rep and update timing state
  sensor->readSensor(sensor);
  lastRepStarted = millis();
  completedReps++;

  // Reps are finished. Reset and allow new command
  if (completedReps >= repeats) 
  {
    busy = false;
    completedReps = 0;
    Serial.println("Executed");
  }
}
