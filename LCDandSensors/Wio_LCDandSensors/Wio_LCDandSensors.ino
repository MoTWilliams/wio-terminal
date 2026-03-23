/*
  Morgan Williams
  CSCE 3612.001
  Lab 3: Wio LCD and Sensors
  Apr 2, 2026
*/
#include <stdbool.h>  // For boolean type in C
#include <stdio.h>    // For snprintf
#include "LIS3DHTR.h"
#include "TFT_eSPI.h"

#define LED LED_BUILTIN
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
typedef char* (*read_t)(char*, size_t);
struct State {
  read_t readSensor;
  const char* name;
  State* next;
  State* prev;
};

char* be_idle(char* buffer, size_t size);
char* read_TempHumd(char* buffer, size_t size);
char* read_Accel(char* buffer, size_t size);
char* read_Light(char* buffer, size_t size);

State idle = { be_idle, "idle", NULL, NULL };
State temp_humd = { read_TempHumd, "temperature/humidity sensing", NULL, NULL };
State accel = { read_Accel, "acceleration sensing", NULL, NULL };
State light = { read_Light, "light sensing", NULL, NULL };

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

LIS3DHTR<TwoWire> lis; // Accelerometer listener
TFT_eSPI tft; // LCD

void setup() {
  // Initialize serial output to console for debugging
  Serial.begin(115200);

  // initialize top buttons as input
  pinMode(next_b.pin, INPUT_PULLUP);
  pinMode(prev_b.pin, INPUT_PULLUP);
  pinMode(reset_b.pin, INPUT_PULLUP);

  // Initialize temperature/humidity sensor

  // Initialize accelerometer
  lis.begin(Wire1);
  if (!lis)
  {
    // Halt and indicate error
    Serial.println("ERROR: Accelerometer not found");
    while (1)
    {
      blink(LONG);
    }
  }
  lis.setOutputDataRate(LIS3DHTR_DATARATE_25HZ); // Data output rate
  lis.setFullScaleRange(LIS3DHTR_RANGE_2G); // Acceleration range

  // Initialize light sensor
  pinMode(LIGHT_SENSOR, INPUT);

  // Initialize buzzer and LED, and ensure that they are off
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);
  analogWrite(BUZZER, 0);
  digitalWrite(LED, LOW);

  // Initialize LCD screen with green background and backlight on
  tft.begin();
  if (!lis)
  {
    Serial.println("ERROR: LCD not found");
    while (1)
    {
      blink(LONG);
    }
  }
  tft.setRotation(3);
  tft.fillScreen(TFT_GREEN);                // Green background
  tft.setTextColor(TFT_BLACK, TFT_GREEN);   // Black text, green background
  tft.setTextSize(2);
  digitalWrite(LCD_BACKLIGHT, HIGH);

  // Create links for the state machine
  buildStateMachine();
}

void loop() {
  static State* currentState = &idle;
  static int textYval = TEXT_START;
  
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

  // If the screen fills up, clear and start over
  if (textYval + TEXT_HEIGHT > SCREEN_HEIGHT)
  {
    tft.fillScreen(TFT_GREEN);
    textYval = TEXT_START;
  }

  char buffer[BUFFER_SIZE];
  currentState->readSensor(buffer, sizeof(buffer));
  Serial.printf("%s\n",buffer);
  tft.drawString(buffer, 0, textYval);
  delay(50);  // Prevent excessive output
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
char* be_idle(char* buffer, size_t size) {
  snprintf(buffer, size, "idle");
  return buffer;
}

char* read_TempHumd(char* buffer, size_t size) {
  float temp, humd;
  temp = 0;
  humd = 0;

  snprintf(buffer, size, "Temp: %7.2fC, Hum: %5.2f%%", temp, humd);
  return buffer;
}

char* read_Accel(char* buffer, size_t size) {
  float x, y, z;
  x = lis.getAccelerationX();
  y = lis.getAccelerationY();
  z = lis.getAccelerationZ();

  snprintf(buffer, size, "X: %5.2f, Y: %5.2f, Z: %5.2f", x, y, z);
  return buffer;
}

char* read_Light(char* buffer, size_t size) {
  int light = analogRead(LIGHT_SENSOR);

  snprintf(buffer, size, "Light value: %4d", light);
  return buffer;
}

/******************************************************************************
 *                                  OUTPUT                                    *
 ******************************************************************************/
// Beep once
void beep(void) {
  analogWrite(BUZZER, 128);
  delay(250);
  analogWrite(BUZZER, 0);
}

// Blink for indicating error state
void blink(int length) {
  digitalWrite(LED, HIGH);
  delay(length);
  digitalWrite(LED, LOW);
  delay(length);
}

// Toggle LED for debounce debugging
void toggle_LED() {
  // Just initialize once
  static int newState = LOW;

  // Perform toggle
  newState = !newState;
  digitalWrite(LED, newState);
}
