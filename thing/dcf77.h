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
            uint8_t dcf77pin = D8;

            dcf77( String name ) : meisterwerk::core::entity( name ) {
            }
            ~dcf77() {
                if ( isOn ) {
                    isOn = false;
                }
            }

            bool registerEntity() {
                // 5sec sensor checks
                bool ret = meisterwerk::core::entity::registerEntity(
                    10000, core::scheduler::PRIORITY_TIMECRITICAL );
                pinMode( dcf77pin, INPUT_PULLUP );

                DBG( "init dcf77." );
                subscribe( entName + "/time/get" );
                isOn = true;
            }

            int          prevSensorValue = 0;
            virtual void onLoop( unsigned long ticker ) override {
                if ( isOn ) {
                    int sensorValue = digitalRead( dcf77pin );
                    if ( sensorValue == 1 && prevSensorValue == 0 ) {
                        Serial.println( "" );
                    }
                    Serial.print( sensorValue );
                    prevSensorValue = sensorValue;
                }
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                // meisterwerk::core::entity::onReceive( origin, topic, msg );
                DBG( "dcf77:" + topic + "," + msg );
                if ( topic == entName + "/time/get" || topic == "*/time/get" ) {
                }
            }

        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
