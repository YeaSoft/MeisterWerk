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
            char cmd2[128];

            dcf77( String name ) : meisterwerk::core::entity( name, 10000000 ) {
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

            int          lastMin = -2;
            virtual void loop() override {
                bool bUpdate;
                if ( isOn ) {
                    TimeElements tt;
                    time_t       localt = util::msgtime::time_t2local( now() );
                    breakTime( localt, tt );
                    int sr  = tt.Second;
                    int mr  = tt.Minute;
                    bUpdate = false;
                    if ( ( mr - lastMin + 60 ) % 60 > 1 ) {
                        if ( sr > 5 ) { // Update every 2min about 5 secs after full minute
                            bUpdate = true;
                            lastMin = mr;
                        }
                    }

                    if ( bUpdate ) {
                        Wire.beginTransmission( addr );
                        Wire.write( cmd0[0] ); // , 1 );
                        byte error = Wire.endTransmission();
                        if ( error != 0 ) {
                            DBG( "Transmission send error" );
                        }
                        int get = 9 + 20;
                        Wire.requestFrom( addr, get );
                        DBG( "DCF-send" );
                        delay( 50 );
                        int n  = 0;
                        int to = 20;
                        while ( n < get && to > 0 ) {
                            if ( Wire.available() ) {
                                cmd2[n] = Wire.read();
                                DBG( "DCF rcv[" + String( n ) + "]:" + String( (int)cmd2[n] ) );
                                ++n;
                                to = 20;
                            } else {
                                yield();
                                --to;
                            }
                        }
                        if ( to == 0 ) {
                            DBG( "DCF77: Timeout!" );
                        } else {
                            char sbuf[128];
                            sprintf( sbuf, "%02d%02d.%02d.%02d  %02d:%02d:%02d  %d [%d]", cmd2[0], cmd2[1], cmd2[2],
                                     cmd2[3], cmd2[4], cmd2[5], cmd2[6], cmd2[7], cmd2[8] );
                            String s = "";
                            for ( int i = 0; i < 20; i++ ) {
                                s += String( (int)cmd2[i + 9] ) + " ";
                            }
                            DBG( s );
                            log( T_LOGLEVEL::INFO, s );
                            log( T_LOGLEVEL::INFO, String( sbuf ) );
                            DBG( "DCF77: received!" );
                        }
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
