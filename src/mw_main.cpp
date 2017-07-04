#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#include "../lib/mw-basicnet.h"

String application="MeisterWerk";

MW_BasicNet mwBN(application, true);

void setup() {
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    mwBN.begin();
}

unsigned int ctr = 0;
unsigned int blfreq=10000;
void loop() { // non-blocking event loop
    ++ctr;

    if (mwBN.handleCom()) {
        // connected to local network
        blfreq=20000;
    } else {
        blfreq=10000;
        // configuration ongoing, waiting.
    }
    if (ctr % blfreq == 0)
        digitalWrite(LED_BUILTIN, LOW); // Turn the LED on
    if (ctr % (blfreq*2) == 0)
        digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off
    if (ctr > (blfreq*3))
        ctr = 0;

}
