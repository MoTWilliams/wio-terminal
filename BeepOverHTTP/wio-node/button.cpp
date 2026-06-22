#include <Arduino.h>
#include "button.h"
#include <stdbool.h>

#include "buzzer.h"
#include "comm.h"

typedef enum {
        WAITING,
        BOUNCING,
        TRIGGERED
} ButtonStates;

static const int BUTTON_PIN = WIO_KEY_C;
static const unsigned long DEBOUNCE_DELAY = 100;

static unsigned long whenTriggered = 0;
static volatile int buttonState = WAITING;

static void isr(void) { buttonState = BOUNCING; }

void button_init(void) {
        // Atach interrupt listener to button press
        pinMode(BUTTON_PIN, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), isr, FALLING);
}

void button_update() {
        if (buttonState == WAITING) return;
        
        if (buttonState == BOUNCING)
        {
                // Ignore triggering on button release
                if (digitalRead(BUTTON_PIN) == HIGH)
                {
                        buttonState = WAITING;
                        return;
                }
                
                // Ignore presses within the debounce window
                unsigned long now = millis();
                if (now - whenTriggered < DEBOUNCE_DELAY)
                {
                        buttonState = WAITING;
                        return;
                }

                // Register valid press
                whenTriggered = now;
                buttonState = TRIGGERED;
                return;
        }
        
        if (buttonState == TRIGGERED)
        {
                // Perform the button action
                client_POST("/api/bark/web", "Bark requested");
                
                // Go back to listening
                buttonState = WAITING;
                return;
        }
}
