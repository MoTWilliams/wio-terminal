/*
  Morgan Williams - mtw0067
  CSCE 3612.001
  Lab 2: Wio Intro
  Feb 19, 2026
*/
#include <stdbool.h>              // For using bool type in C

#define LED LED_BUILTIN
#define BUZZER WIO_BUZZER

#define DEBOUNCE_DELAY 50
#define ODD0 1
#define ODD1 3
#define FIB0 1
#define FIB1 1

typedef struct Button {
  const int pin;                  // Physical button
  int read;                       // Current raw button reading
  int lastRead;                   // Previous raw button reading
  int stable;                     // Most recent stable button state
  unsigned long lastDebounceTime; // Timestamp of last raw state change
} Button;

// HIGH = button in released state
Button odd = { WIO_KEY_A, HIGH, HIGH, HIGH, 0 };  // Right
Button fib = { WIO_KEY_B, HIGH, HIGH, HIGH, 0 };  // Middle
Button res = { WIO_KEY_C, HIGH, HIGH, HIGH, 0 };  // Left


typedef struct Seq Seq;
typedef int (*func_t)(Seq* s);    // Used instead of switch or if-statement to
                                  // select the corresponding recurrence
                                  // relation in get_next()

struct Seq {
  const char* name;
  const int a0, a1;
  int i, prev, curr;
  const func_t step;    // Pointer to the corresponding recurrence relation
};

// Recurrence relations
int odd_function(Seq* s) { return 2 * s->curr - s->prev; }
int fib_function(Seq* s) { return s->curr + s->prev; }

// Reset counts for one sequence
void reset(Seq* s) {
  s->i = 0;
  s->prev = s->a0;
  s->curr = s->a1;
}

// Package recurrence relation and important values for each sequence
Seq oddVals = { "odd", ODD0, ODD1, 0, ODD0, ODD1, odd_function };
Seq fibVals = { "fibonacci", FIB0, FIB1, 0, FIB0, FIB1, fib_function };


// Runs once
void setup() {
  // Initialize serial output to console for debugging
  Serial.begin(115200);

  // initialize top buttons as input
  pinMode(res.pin, INPUT_PULLUP);
  pinMode(odd.pin, INPUT_PULLUP);
  pinMode(fib.pin, INPUT_PULLUP);

  // Initialize buzzer and LED as output
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  // Begin with buzzer and LED off
  digitalWrite(LED, LOW);
  analogWrite(BUZZER, 0);
}

// Runs thousands of times a second, unless delayed
void loop() {
  int blinks = 0;

  // Button A
  if (button_pressed(&odd))
  {
    // Beep once to indicate button pressed, then get the next odd value
    beep();
    blinks = get_next(&oddVals);
  }

  // Button B
  else if (button_pressed(&fib))
  {
    // Beep once to indicate button pressed, then get next fibonacci value
    beep();
    blinks = get_next(&fibVals);
  }

  // Button C
  else if (button_pressed(&res))
  {
    // Just reset all values
    reset_all();
  }

  // Do blinks
  blink(blinks);
}


// Stop switch bouncing
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


// Button actions
void reset_all(void) {
  reset(&oddVals);
  reset(&fibVals);
  
  Serial.printf("Reset all values\n");
}

int get_next(Seq* s) {
  // Note in the report that you initially forgot to show a0 and a1 before 
  // starting the sequence
  int next;

  // Don't skip the first two values
  if (s->i == 0) next = s->a0;
  else if (s->i == 1) next = s->a1;
  else
  {
    // Otherwise, calculate the next value.
    next = s->step(s);

    s->prev = s->curr;
    s->curr = next;
  }

  Serial.printf("Got %s value a%d = %d\n", s->name, s->i, next);
  s->i++;
  return next;
}


// Output
void beep(void) {
  analogWrite(BUZZER, 128);
  delay(250);
  analogWrite(BUZZER, 0);
}

void blink(int ct) {
  // Force the LED off first
  digitalWrite(LED, LOW);
  
  // Do nothing if ct == 0
  if (ct <= 0) return;
  
  // Otherwise, perform blinks
  for (int i = 0; i < ct; i++)
  {
    digitalWrite(LED, HIGH);
    delay(300);
    digitalWrite(LED, LOW);
    delay(200);
  }
}
