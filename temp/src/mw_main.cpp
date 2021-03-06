#include <vector>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>

// Install PubSubClient with:
// pio lib search PubSubClient  # -> library ID
// pio lib install <ID>
#include <PubSubClient.h>

#include "../lib/mw-queue.h"
#include "../lib/mw-entity.h"
#include "../lib/mw-scheduler.h"

#include "../lib/mw-basicnet.h"
#include "../lib/mw-mqtt.h"

#include "../lib/entities/mw-ent-led.h"

// Application name, used for default hostnames and AP-names
String application = "MeisterWerk";

// Task Scheduler object
MW_Scheduler mwScheduler;

// Basic networking and web config
MW_BasicNet mwBN(application, 20, true); // application-name, timeout-for-connecting-to-AP, serial-debug-active

// MQTT client
MW_MQTT mwMQ;

// EEPROM user configuration content
T_EEPROM tep;

// New led 
MW_Led onboardLed("OnboardLed", BUILTIN_LED, false);  // true: publish all led changes...

// START SCHROTTHAUFEN -> Entity-Object-Model needed.
//--- Button task --------------------------------------------------------------
int resetButtonDurationPressedMs = 0;
int resetButtonState = MW_STATE_UNDEFINED;
int lastButtonState = MW_STATE_UNDEFINED;
int lastButtonTicker = 0;
int buttonMode=0;
unsigned int buttonPort;
unsigned int resetButtonGetState()
{
    int state = digitalRead(buttonPort);
    if (state == 0)
        resetButtonState = MW_STATE_ON;
    else
        resetButtonState = MW_STATE_OFF;
    return resetButtonState;
}
void resetButtonInit(unsigned int port)
{
    buttonPort = port;
    pinMode(buttonPort, INPUT);
    lastButtonState = resetButtonGetState();
}
void resetButtonLoop(unsigned long ticker)
{
    if (lastButtonTicker == 0)
        lastButtonTicker = ticker;
    unsigned long deltaMs = (ticker - lastButtonTicker) / 1000L;
    lastButtonTicker=ticker;
    if (resetButtonGetState() == MW_STATE_OFF)
    {
        buttonMode=0;
        if (resetButtonDurationPressedMs>0)
            {
                Serial.println("ResetDurationMs: "+String(resetButtonDurationPressedMs));
                if (resetButtonDurationPressedMs >= 30000)
                {
                    // >=30 sec Long press: kill config eeprom
                    resetButtonDurationPressedMs = 0;
                    Serial.println("WARNING: HARD FACTORY RESET INITIATED!");
                    Serial.println("EEPROM will be erased. The UUID of this device will be regenerated.");
                    mwBN.clearEEPROM();
                    Serial.println("EEPROM erased, restarting...");
                    ESP.restart();
                }
                else
                {
                    if (resetButtonDurationPressedMs >= 10000)
                    {
                        // >= 10 < 30 sec: go into access point mode: (XXX: hacky)
                        resetButtonDurationPressedMs = 0;
                        Serial.println("Entering accesspoint mode... (no restart)");
                        mwBN.enterAccessPointMode();
                        onboardLed.setBlinkIntervallMs(500); // XXX: should be set by entering access point mode
                    }
                    else
                    {
                        // <10 sec short press, restart esp()
                        resetButtonDurationPressedMs = 0;
                        Serial.println("Soft restart...");
                        ESP.restart();
                    }
                }
            }
    }
    else
    { // Button is pressed.
        resetButtonDurationPressedMs += deltaMs;
        if (resetButtonDurationPressedMs >= 30000)
        {                                // XXX: set only once
            if (buttonMode<2) {
                Serial.println("EEPROM erasure mode entered.");
                onboardLed.setBlinkIntervallMs(100); // Furious blinking
                buttonMode=2;
            }
        }
        else
        {
            if (resetButtonDurationPressedMs >= 10000)
            {
                if (buttonMode<1) {
                    Serial.println("Enter AP mode (no erase config)");
                    onboardLed.setBlinkIntervallMs(300); // Fast blinking
                    buttonMode=1;
                }
            }
        }
    }
}
//--- Heap watchdog task ------------------------------------------------------
extern "C" {
#include "user_interface.h"
}
unsigned long getFreeHeap()
{
    uint32_t free = system_get_free_heap_size();
    return (unsigned long)free;
}
void watchdogLoop(unsigned long ticker)
{
    unsigned long freeHeap = getFreeHeap();
    if (freeHeap < 10000L)
    {
        // Alarm!
        Serial.println("Watchdog: heap is low: " + String(freeHeap));
    }
}
//--- Basic Networking & Web config -------------------------------------------
bool isWifiConnected = false;
void basicNetworkingWebConfigLoop(unsigned long ticker) {
    // Handle AP connection/creation and Web config interface.
    isWifiConnected = mwBN.handleCom();
}
//--- MQTT client -------------------------------------------------------------
void mqttClientLoop(unsigned long ticker) {
    if (isWifiConnected)
    {
        // connected to local network
        onboardLed.setBlinkIntervallMs(2000);
        mwMQ.handleMQTT();
    }
}
//-----------------------------------------------------------------------------
// END SCHROTTHAUFEN ----------------------------------------------------------

void setup()
{
    // Debug console
    Serial.begin(115200);

    // Internal LED
    onboardLed.setState(MW_STATE_ON);
    onboardLed.setMode(LED_MODE_BLINK);
    onboardLed.setBlinkIntervallMs(500); // XXX: should be set by entering access point mode

    // Reset button
    resetButtonInit(GPIO_ID_PIN0); // ID_PIN0 == D3 is the Flash button and will be observed for soft/hard reset.
                                   // short press (<10sec): software-reboot,
                                   // long press (10-30sec): enter access point mode, keep config (fast blinking)
                                   // very long press (>=30 sec): erase eeprom, factory reset. (furious blinking)
    mwScheduler.addTask("ResetButton", resetButtonLoop, 50000L, MW_PRIORITY_SYSTEMCRITICAL);

    // Heap watchdog
    mwScheduler.addTask("Watchdog", watchdogLoop, 2000000L, MW_PRIORITY_LOW);

    // Basic networking & web configuration for accesspoint & WiFi-client mode
    mwBN.begin(); // Start MW Basic Networking: try to connect to AP, or on failure: create config AP (http://10.1.1.1)
    mwScheduler.addTask("BasicNetworking", basicNetworkingWebConfigLoop, 10000L, MW_PRIORITY_HIGH);

    // EEPROM configuration    
    tep = mwBN.getEEPROM();

    // MQTT client
    mwMQ.begin(tep.mqttserver);
    mwScheduler.addTask("MqttClient", mqttClientLoop, 20000L, MW_PRIORITY_HIGH);
}

void loop()
{ // non-blocking event loop
    mwScheduler.loop();
}
