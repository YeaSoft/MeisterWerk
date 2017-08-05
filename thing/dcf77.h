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
            bool isOn = false;
            char addr = 0x6e;
            char cmd0[2];
            char cmd2[16];

            dcf77( String name ) : meisterwerk::core::entity( name, 1000000 ) {
            }
            ~dcf77() {
                if ( isOn ) {
                    isOn = false;
                }
            }

            virtual void setup() override {
                DBG( "init dcf77." );
                cmd0[0] = 0;
                cmd0[1] = 2;
                subscribe( entName + "/time/get" );
                subscribe( "time/get" );
                isOn = true;
            }

            virtual void loop() override {
                if ( isOn ) {
                    Wire.beginTransmission( addr );
                    Wire.write( cmd0[0] ); // , 1 );
                    byte error = Wire.endTransmission();
                    if ( error != 0 ) {
                        DBG( "Transmission send error" );
                    }
                    Wire.requestFrom( addr, 9 );
                    DBG( "DCF-send" );
                    delay( 50 );
                    int n  = 0;
                    int to = 20;
                    while ( n < 9 && to > 0 ) {
                        if ( Wire.available() ) {
                            cmd2[n] = Wire.read();
                            DBG( "DCF rcv[" + String( n ) + "]:" + String( (int)cmd2[n] ) );
                            ++n;
                        } else {
                            delay( 10 );
                            --to;
                        }
                    }
                    if ( to == 0 ) {
                        DBG( "DCF77: Timeout!" );
                    } else {
                        DBG( "DCF77: received!" );
                    }
                }
            }

            virtual void receive( const char *origin, const char *ctopic, const char *msg ) override {
                // meisterwerk::core::entity::receive( origin, topic, msg );
                String topic( ctopic );
                DBG( "dcf77:" + topic + "," + msg );
                if ( topic == entName + "/time/get" || topic == "time/get" ) {
                }
            }

        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
