/*
  Morgan Williams
  CSCE 3612.001
  Lab 3: Wio LCD and Sensors
  Apr 2, 2026
*/
#include <stdbool.h>  // For boolean type in C
#include <stdio.h>    // For snprintf
#include <Wire.h>
#include "LIS3DHTR.h" // Accelerometer
#include "TFT_eSPI.h" // LCD
#include "AHT20.h"    // Temp/humidity sensor

#define BUZZER WIO_BUZZER
#define LIGHT_SENSOR WIO_LIGHT

#define DEBOUNCE_DELAY 50
#define NUM_SENSORS 3

#define LONG 1000
#define SHORT 250
#define BUFFER_SIZE 64

#define TEXT_START 1
#define TEXT_HEIGHT 17
#define SCREEN_HEIGHT 240

// PUSH-BUTTONS

typedef struct Button {
  const int pin;                  // Physical button
  int read;                       // Current raw button reading
  int lastRead;                   // Previous raw button reading
  int stable;                     // Most recent stable button state
  unsigned long lastDebounceTime; // Timestamp of last raw state change
} Button;

// HIGH = button in released state
Button next_b = { WIO_KEY_A, HIGH, HIGH, HIGH, 0 };  // Right
Button prev_b = { WIO_KEY_B, HIGH, HIGH, HIGH, 0 };  // Middle
Button reset_b = { WIO_KEY_C, HIGH, HIGH, HIGH, 0 }; // Left

// SENSING STATES

typedef struct State State;
typedef void (*read_t)(State*, char*, size_t);
struct State {
  read_t readSensor;
  const char* name;
  State* next;
  State* prev;
};

void be_idle(State*, char* buffer, size_t size);
void read_NoSensor(State* state, char* buffer, size_t size);
void read_TempHumd(State*, char* buffer, size_t size);
void read_Accel(State*, char* buffer, size_t size);
void read_Light(State*, char* buffer, size_t size);

State idle = { be_idle, "idle", NULL, NULL };
State temp_humd = { read_TempHumd, "temp/humd sensor", NULL, NULL };
State accel = { read_Accel, "accelerometer", NULL, NULL };
State light = { read_Light, "light sensor", NULL, NULL };

void buildStateMachine() {
  idle.next = &temp_humd;
  idle.prev = &light;

  temp_humd.next = &accel;
  temp_humd.prev = &light;

  accel.next = &light;
  accel.prev = &temp_humd;

  light.next = &temp_humd;
  light.prev = &accel;
}

AHT20 aht;                // Temp/humidity sensor
LIS3DHTR<TwoWire> lis;    // Accelerometer
TFT_eSPI tft;             // LCD

void setup() {
  // OUTPUT

  // Initialize serial output to console for debugging
  Serial.begin(115200);
  
  // Initialize buzzer, and ensure that it is off
  pinMode(BUZZER, OUTPUT);
  analogWrite(BUZZER, 0);

  // Initialize LCD screen with green background and backlight on
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_GREEN);                // Green background
  tft.setTextColor(TFT_BLACK, TFT_GREEN);   // Black text, green background
  tft.setTextSize(2);
  digitalWrite(LCD_BACKLIGHT, HIGH);

  // Create links for the state machine
  buildStateMachine();

  // USER INPUT

  // initialize top buttons as input
  pinMode(next_b.pin, INPUT_PULLUP);
  pinMode(prev_b.pin, INPUT_PULLUP);
  pinMode(reset_b.pin, INPUT_PULLUP);

  // SENSOR INPUT

  // Initialize temperature/humidity sensor
  aht.begin();
  // Check connection by attempting to read from the sensor. Continue without
  // input if sensor not found.
  float _h, _t;
  if (!aht.getSensor(&_h, &_t)) temp_humd.readSensor = read_NoSensor;

  // Initialize accelerometer
  lis.begin(Wire1);
  // Continue without accelerometer input if not found
  if (!lis) accel.readSensor = read_NoSensor;
  else
  {
    lis.setOutputDataRate(LIS3DHTR_DATARATE_25HZ);    // Data output rate
    lis.setFullScaleRange(LIS3DHTR_RANGE_2G);         // Acceleration range
  }

  // Initialize light sensor
  pinMode(LIGHT_SENSOR, INPUT);
}

void loop() {
  static int textYval = TEXT_START;
  static State* currentState = &idle;

  // Button presses trigger state transitions

  // Button A--cycle forward
  if (button_pressed(&next_b))
  {
    beep();
    // Overwrite "idle"
    if (currentState != &idle) textYval += TEXT_HEIGHT;
    currentState = currentState->next;
  }
  
  // Button B--cycle backward
  else if (button_pressed(&prev_b))
  {
    beep();
    // Overwrite "idle"
    if (currentState != &idle) textYval += TEXT_HEIGHT;
    currentState = currentState->prev;
  }

  // Button C--clear screen and start from idle
  else if (button_pressed(&reset_b))
  {
    beep();
    currentState = &idle;
    tft.fillScreen(TFT_GREEN);
    textYval = TEXT_START;
  }

  updateLCD(currentState, &textYval);
}

/******************************************************************************
 *                              INPUT HANDLING                                *
 ******************************************************************************/
// Stop switch bounce
// Based on: https://docs.arduino.cc/built-in-examples/digital/Debounce/
bool button_pressed(Button* b) {
  bool pressed = false;
  b->read = digitalRead(b->pin);

  // Grab a timestamp if the button has changed state
  if (b->read != b->lastRead) b->lastDebounceTime = millis();

  // Ignore state changes for 50ms
  if (millis() - b->lastDebounceTime > DEBOUNCE_DELAY)
  {
    // If it's been long enough, register the button press
    if (b->read != b->stable)
    {
      b->stable = b->read;
      // Only on press, not release
      if (b->stable == LOW) pressed = true;
    }
  }

  b->lastRead = b->read;
  return pressed;
}

/******************************************************************************
 *                               DATA_CAPTURE                                 *
 ******************************************************************************/
void be_idle(State*, char* buffer, size_t size) {
  snprintf(buffer, size, "idle");
}

void read_NoSensor(State* state, char* buffer, size_t size) {
  snprintf(buffer, size, "%s not found", state->name);
}

// Temp/humidity sensor read takes 80ms (blocking), no matter what:
// https://www.aosong.com/userfiles/files/media/Data%20Sheet%20AHT20.pdf p.12
void read_TempHumd(State*, char* buffer, size_t size) {
  const int READ_INTERVAL = 2000; // 2 seconds

  static unsigned long lastRead = 0;
  static char cacheBuffer[BUFFER_SIZE] = "   No temp/humidity yet    ";
  float humd, temp;

  // Only read every 2 seconds
  if (millis() - lastRead >= READ_INTERVAL)
  {
    lastRead = millis();

    if (!aht.getSensor(&humd, &temp))
      snprintf(cacheBuffer, sizeof(cacheBuffer), " Temp/humidity read failed ");
    else
      snprintf(cacheBuffer, sizeof(cacheBuffer), 
                "Temp: %7.2fC, Hum: %5.2f%%", temp, humd*100 );
  }
  
  snprintf(buffer, size, "%s", cacheBuffer);
}

void read_Accel(State*, char* buffer, size_t size) {
  float x, y, z;
  x = lis.getAccelerationX();
  y = lis.getAccelerationY();
  z = lis.getAccelerationZ();

  snprintf(buffer, size, "X: %5.2f Y: %5.2f Z: %5.2f", x, y, z);
}

void read_Light(State*, char* buffer, size_t size) {
  int light = analogRead(LIGHT_SENSOR);

  snprintf(buffer, size, "Light value: %4d", light);
}

/******************************************************************************
 *                                  OUTPUT                                    *
 ******************************************************************************/
// Handle LCD output
void updateLCD(State* state, int* textYval) {
  static unsigned long lastUpdate = 0; // Time of last update
  const unsigned long UPDATE_INTERVAL = 100;

  // If the screen fills up, clear and start over
  if (*textYval + TEXT_HEIGHT > SCREEN_HEIGHT)
  {
    tft.fillScreen(TFT_GREEN);
    *textYval = TEXT_START;
  }

  // Update every 100 ms
  if (millis() - lastUpdate >= UPDATE_INTERVAL)
  {
    lastUpdate = millis();

    char buffer[BUFFER_SIZE];
    state->readSensor(state, buffer, sizeof(buffer));
    Serial.printf("%s\n",buffer);
    tft.drawString(buffer, 0, *textYval);
  }
}

// Beep once
void beep(void) {
  analogWrite(BUZZER, 128);
  delay(250);
  analogWrite(BUZZER, 0);
}

