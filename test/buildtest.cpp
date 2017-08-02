// buildtest.cpp - for automated build tests

// platform includes
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <Arduino.h>
#endif

#define DEBUG 1

// framework includes
#include <MeisterWerk.h>

using namespace meisterwerk;

// application class
class TestApp : public core::baseapp {
    public:
    TestApp() : core::baseapp( "TestApp" ) {
    }

    virtual void setup() override {
        // Debug console
        Serial.begin( 115200 );
        Serial.println( "Horray!\n" );
    }
};

// Application Instantiation
TestApp app;
