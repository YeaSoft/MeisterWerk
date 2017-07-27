// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <ESP8266WebServer.h>
//#include <ESP8266WiFi.h>
//#include <WiFiClient.h>

#include <SoftwareSerial.h>

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../base/i2cdev.h"
#include "../core/entity.h"
#include "../util/hextools.h"
#include "../util/metronome.h"
#include "../util/msgtime.h"

namespace meisterwerk {
    namespace thing {
        class dcf77 : public meisterwerk::core::entity {
            public:
            bool    isOn     = false;
            uint8_t dcf77pin = D5; // D8 needs to be low on boot, don't use.

            dcf77( String name ) : meisterwerk::core::entity( name ) {
            }
            ~dcf77() {
                if ( isOn ) {
                    isOn = false;
                }
            }

            bool registerEntity() {
                // 5sec sensor checks
                bool ret = meisterwerk::core::entity::registerEntity( 20000, core::scheduler::PRIORITY_TIMECRITICAL );
                pinMode( dcf77pin, INPUT ); // INPUT_PULLUP );

                DBG( "init dcf77." );
                subscribe( entName + "/time/get" );
                isOn = true;
            }

            int           prevSensorValue = 0;
            unsigned long lasttick        = 0;
            virtual void onLoop( unsigned long ticker ) override {
                if ( isOn ) {
                    if ( lasttick == 0 )
                        lasttick    = millis();
                    int sensorValue = digitalRead( dcf77pin );
                    if ( sensorValue != prevSensorValue ) {
                        DBG( "DCF-bit " + String( prevSensorValue ) + ", duration: " + String( millis() - lasttick ) );
                        prevSensorValue = sensorValue;
                        lasttick        = millis();
                    }
                }
            }

            virtual void onReceive( const char *origin, const char *ctopic, const char *msg ) override {
                // meisterwerk::core::entity::onReceive( origin, topic, msg );
                String topic( ctopic );
                DBG( "dcf77:" + topic + "," + msg );
                if ( topic == entName + "/time/get" || topic == "*/time/get" ) {
                }
            }

        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
