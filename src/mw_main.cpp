#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Install PubSubClient with:
// pio lib search PubSubClient  # -> library ID
// pio lib install <ID>
#include <PubSubClient.h>

#include "../lib/mw-scheduler.h"

#include "../lib/mw-basicnet.h"
#include "../lib/mw-mqtt.h"

String application = "MeisterWerk";

MW_Scheduler mwScheduler;

MW_BasicNet mwBN(application, 20, true); // application-name, timeout-for-connecting-to-AP, serial-debug-active
MW_MQTT mwMQ;
T_EEPROM tep;

//--- LED task --------------
#define LED_MODE_STATIC 0
#define LED_MODE_BLINK 1
unsigned int ledMode=LED_MODE_STATIC;
unsigned int ledState=0;      // 0: off, 1: on;
unsigned int ledBlinkIntervallMs=0;
unsigned long lastChange;
unsigned int ledPort;
void ledInit(unsigned int port) {
    ledPort=port;
    pinMode(ledPort, OUTPUT);
}
void ledSetMode(unsigned int mode) { // LED_MODE_{STATIC|BLINK}
    ledMode=mode;
}
void ledSetState(unsigned int state) {
    ledState=state;
    if (ledState==0) digitalWrite(ledPort, HIGH); // Turn the LED off
    else digitalWrite(ledPort, LOW); // Turn the LED on
}
void ledSetBlinkIntervallMs(unsigned int intervall) {
    ledBlinkIntervallMs=intervall;
}
void ledLoop(unsigned long ticker) {
    // Serial.println("ledLoop: "+String(ticker));
    if (ledMode==LED_MODE_BLINK) {
        if ((lastChange-ticker)/1000L > ledBlinkIntervallMs) {
            lastChange=ticker;
            if (ledState==0) {
                ledSetState(1);
            } else {
                ledSetState(0);
            }
        }
    }
}
//--- Reset task ------------
//---------------------------

void setup()
{
    Serial.begin(9600);

    ledInit(BUILTIN_LED);
    
    pinMode(GPIO_ID_PIN0, INPUT); // ID_PIN0 == D3 is the Flash button and will be observed for soft/hard reset.

    mwBN.begin(); // Start MW Basic Networking: try to connect to AP, or on failure: create config AP (http://10.1.1.1).
    tep = mwBN.getEEPROM();

    mwMQ.begin(tep.mqttserver);

    mwScheduler.addTask("InternalLed",ledLoop,100000L,1);
    ledSetMode(LED_MODE_BLINK);
    ledSetBlinkIntervallMs(500);
}
extern "C" {
#include "user_interface.h"
}
unsigned int observeHeap()
{
    uint32_t free = system_get_free_heap_size();
    return (unsigned int)free;
}

unsigned int tickfreq = 10000;
unsigned int tickctr = 0;
int softrstctr = 0;

bool isConnected = false;
int rstButton;

void loop()
{ // non-blocking event loop
    mwScheduler.loop();

    ++tickctr;

    // Handle AP connection/creation and Web config interface.
    isConnected = mwBN.handleCom();

    if (isConnected)
    {
        // connected to local network
        ledSetBlinkIntervallMs(1500);
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
                ledSetBlinkIntervallMs(200);
                // blfreq = 6000; // very fast blinking on iminent hard reset.
            }
        }
    }

}
