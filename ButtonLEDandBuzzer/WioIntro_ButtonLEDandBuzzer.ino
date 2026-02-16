/*
  Morgan Williams - mtw0067
  CSCE 3612.001
  Lab 2: Wio Intro
  Feb 19, 2026
*/
#include <stdbool.h>

#define LED LED_BUILTIN
#define BUZZER WIO_BUZZER

#define DEBOUNCE_DELAY 50

typedef struct Button {
  int pin;
  int read;                       // Current pre-debounce button reading
  int lastRead;                   // Last pre-debounce button reading
  int stable;                     // Debounced button state
  unsigned long lastDebounceTime; // lastRaw's timestamp
  int count;
} Button;

Button res = {WIO_KEY_C, HIGH, HIGH, HIGH, 0, 0};
Button fun = {WIO_KEY_A, HIGH, HIGH, HIGH, 0, 0};
Button fib = {WIO_KEY_B, HIGH, HIGH, HIGH, 0, 0};

int ledState = LOW;

void setup() {
  // initialize top buttons as input
  Serial.begin(115200);
  pinMode(res.pin, INPUT_PULLUP);
  pinMode(fun.pin, INPUT_PULLUP);
  pinMode(fib.pin, INPUT_PULLUP);

  // Initialize buzzer and LED as output
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  // Begin with buzzer and LED off
  digitalWrite(LED, LOW);
  analogWrite(BUZZER, 0);
}

void loop() {
  if (button_debounce(&res)) ledState = !ledState;
  if (button_debounce(&fun)) ledState = !ledState;
  if (button_debounce(&fib)) ledState = !ledState;

  digitalWrite(LED, ledState);
}

// Based on: https://docs.arduino.cc/built-in-examples/digital/Debounce/
bool button_debounce(Button* b) {
  bool pressed = false;
  b->read = digitalRead(b->pin);

  // Grab a timestamp if the button has changed state
  if (b->read != b->lastRead) b->lastDebounceTime = millis();

  // Ignore more state changes for 50ms
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









void reset_counts()
{
  for (int i = 0; i < 3; i++)
  {
    analogWrite(WIO_BUZZER, 128);
    delay(250);
    analogWrite(WIO_BUZZER, 0);
    delay(250);
  }

  fun.count = 0;
  fib.count = 0;
}

void do_blinks(int ct)
{
  for (int i = 0; i < ct; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }
}
