#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Install PubSubClient with:
// pio lib search PubSubClient  # -> library ID
// pio lib install <ID>
#include <PubSubClient.h>

#include "../lib/mw-basicnet.h"
#include "../lib/mw-mqtt.h"

String application = "MeisterWerk";

MW_BasicNet mwBN(application, 20, true); // application-name, timeout-for-connecting-to-AP, serial-debug-active
MW_MQTT mwMQ;
T_EEPROM tep;

void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(GPIO_ID_PIN0, INPUT); // ID_PIN0 == D3 is the Flash button and will be observed for soft/hard reset.

    mwBN.begin(); // Start MW Basic Networking: try to connect to AP, or on failure: create config AP (http://10.1.1.1).
    tep = mwBN.getEEPROM();

    mwMQ.begin(tep.mqttserver);
}
extern "C" {
#include "user_interface.h"
}
unsigned int observeHeap()
{
    uint32_t free = system_get_free_heap_size();
    return (unsigned int)free;
}
unsigned int ctr = 0;
unsigned int blfreq = 10000; // faster blinky during connection & configuration phase.

unsigned int tickfreq = 10000;
unsigned int tickctr = 0;
int softrstctr = 0;

bool isConnected = false;
int rstButton;

void loop()
{ // non-blocking event loop
    ++ctr;
    ++tickctr;

    // Handle AP connection/creation and Web config interface.
    isConnected = mwBN.handleCom();

    if (isConnected)
    {
        // connected to local network
        blfreq = 20000; // slower blinky on normal operation
        mwMQ.handleMQTT();
    }

    if (tickctr >= tickfreq) // check every tickfreq loops, if button is (still) pressed.
    {
        tickctr = 0;
        rstButton = digitalRead(GPIO_ID_PIN0);
        if (rstButton != 0)
        { // Button not pressed.
            if (softrstctr > 0) // Button was pressed just up to now.
            { // SoftRST button released
                if (softrstctr > 30)
                {
                    // Long press: kill config eeprom
                    Serial.println("WARNING: HARD FACTORY RESET INITIATED!");
                    Serial.println("EEPROM will be erased. The UUID of this device will be regenerated.");
                    mwBN.clearEEPROM();
                    Serial.println("EEPROM erased, restarting...");
                    ESP.restart();
                }
                else
                {
                    // short press, restart esp()
                    Serial.println("Soft restart...");
                    ESP.restart();
                }
            }
        }
        else
        { // Button is begin pressed, increase duration counter
            ++softrstctr;
            if (softrstctr == 1)
                Serial.println("SoftRST button is being pressed...");
            if (softrstctr > 30)
            {
                if (softrstctr == 31)
                    Serial.println("HARD-RESET duration reached...");
                blfreq = 6000; // very fast blinking on iminent hard reset.
            }
        }
    }

    if (ctr % blfreq == 0)
    {
        digitalWrite(LED_BUILTIN, LOW); // Turn the LED on
        //Serial.println(observeHeap());
    }
    if (ctr % (blfreq * 2) == 0)
        digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off
    if (ctr > (blfreq * 3))
        ctr = 0;
}
