/*
        CLEAN COMPILE WORKAROUND, because Arduino IDE is using a stale cached 
        .ino.cpp.o for Wifi-related sketches. If changes don't appear to 
        register, compile from PowerShell with the following commands:

        $WIO_PROJECT = "C:\dev\projects\Arduino\wio-terminal\BeepOverWifi"
        $ARDUINO_LIBS = "C:\dev\projects\Arduino\libraries"
        arduino-cli compile --clean --fqbn Seeeduino:samd:seeed_wio_terminal \
                            --libraries $ARDUINO_LIBS $WIO_PROJECT
    
        Then upload in the IDE
*/

#include "connection.h"
#include "buzzer.h"

void setup() {
        Serial.begin(115200);
        while(!Serial);

        buzzer_init();
        buzzer_beep();

        wifi_init();
        server_init();
}

void loop() {
        server_listen();
}
