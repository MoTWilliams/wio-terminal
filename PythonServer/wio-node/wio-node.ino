#include <rpcWiFi.h>
#include "arduino_secrets.h"

// https://wiki.seeedstudio.com/Wio-Terminal-Wi-Fi/#wi-fi-client-example-code

void setup() {
        Serial.begin(115200);
        while(!Serial); // Wait for Serial to be ready

        // Set WiFi to station mode and disconnect from an AP if it was
        // previously connected
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(2000);

        WiFi.begin(SSID, PASSWORD);

        while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.println("Connecting to WiFi.."); 
        }
        Serial.println("Connected to the WiFi network");
        Serial.print("IP Address: ");
        Serial.println (WiFi.localIP()); // prints out the device's IP address
}

void loop() {
        Serial.print("Connecting to ");
        Serial.println(HOST);

        // Use WiFiClient class to create TCP connections
        WiFiClient client;

        if (!client.connect(HOST, PORT)) {
                Serial.println("Connection failed.");
                Serial.println("Waiting 5 seconds before retrying...");
                delay(5000);
                return;
        }

        // This will send a request to the server
        //uncomment this line to send an arbitrary string to the server
        //client.print("Send this data to the server");
        //uncomment this line to send a basic document request to the server
        client.print("GET / HTTP/1.1\n\n"); // sending HTTP GET request

        int maxloops = 0;

        //wait for the server's reply to become available
        while (!client.available() && maxloops < 1000) {
                maxloops++;
                delay(1); //delay 1 msec
        }

        if (client.available() > 0) {
                //read back one line from the server
                String line = client.readString(); // Read from the server response
                // Proceed various line-endings
                line.replace("\r\n", "\n");
                line.replace('\r', '\n');
                line.replace("\n", "\r\n");
                Serial.println(line);
        } else {
                Serial.println("client.available() timed out ");
        }

        Serial.println("Closing connection.");
        client.stop();

        Serial.println("Waiting 5 seconds before restarting...");
        delay(5000);
}
