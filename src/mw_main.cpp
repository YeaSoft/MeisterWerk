#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Install PubSubClient with:
// pio lib search PubSubClient  # -> library ID
// pio lib install <ID>
#include <PubSubClient.h>

#include "../lib/mw-basicnet.h"
#include "../lib/mw-mqtt.h"

String application="MeisterWerk";

MW_BasicNet mwBN(application, 20, true); // application-name, timeout-for-connecting-to-AP, serial-debug-active
MW_MQTT mwMQ;

void setup() {
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);

    mwBN.begin(); // Start MW Basic Networking: try to connect to AP, or on failure: create config AP (http://10.1.1.1).

    String mqttServer=""; // disable
    mwMQ.begin(mqttServer);
}

unsigned int ctr = 0;
unsigned int blfreq=10000;
bool isConnected=false;
void loop() { // non-blocking event loop
    ++ctr;

    // Handle AP connection/creation and Web config interface.
    isConnected=mwBN.handleCom()) 
    
    if (isConnected)
    {
        // connected to local network
        blfreq=20000; // slow blinky on normal operation
        mwMQ.handleMQTT();
    }

    if (ctr % blfreq == 0)
        digitalWrite(LED_BUILTIN, LOW); // Turn the LED on
    if (ctr % (blfreq*2) == 0)
        digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off
    if (ctr > (blfreq*3))
        ctr = 0;

}
