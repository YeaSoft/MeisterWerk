// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <ESP8266WiFi.h>

#include <SoftwareSerial.h>

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../base/i2cdev.h"
#include "../core/entity.h"
#include "../util/hextools.h"
#include "../util/msgtime.h"

namespace meisterwerk {
    namespace thing {
        class GPS_NEO_6M : public meisterwerk::core::entity {
            public:
            // LiquidCrystal_I2C *plcd;
            // Adafruit_GPS *  pgps;
            SoftwareSerial *pser;
            bool            isOn = false;
            // meisterwerk::util::sensorprocessor tempProcessor, pressProcessor;
            String  json;
            String  dispSize;
            uint8_t adr;
            uint8_t instAddress;
            bool    usingInterrupt = false;

            GPS_NEO_6M( String name ) : meisterwerk::core::entity( name ) {
            }
            ~GPS_NEO_6M() {
                if ( isOn ) {
                    isOn = false;
                    //  delete pgps;
                    delete pser;
                }
            }

            bool registerEntity() {
                // 5sec sensor checks
                bool ret = meisterwerk::core::entity::registerEntity( 50000 );
                DBG( "init gps." );

                pser = new SoftwareSerial( D6, D7, false, 256 ); // RX, TX, inverseLogic, bufferSize
                pser->begin( 9600 );
                resetCmd();
                resetDefaults();
                isOn = true;
            }

            int l = 0;
#define NMEA_MAX_CMDS 32
            String cmd[NMEA_MAX_CMDS];
            int    icmd    = 0;
            String gpstime = "", gpsdate = "", lat = "", lon = "", alt = "", valid = "";
            int    nosat = 0;
            int    fix   = 0;

            void resetCmd() {
                for ( int i = 0; i < NMEA_MAX_CMDS; i++ )
                    cmd[i] = "";
                icmd = 0;
            }

            void resetDefaults() {
                icmd    = 0;
                gpstime = "";
                gpsdate = "";
                lat     = "";
                lon     = "";
                alt     = "";
                valid   = "";
                nosat   = 0;
                fix     = 0;
            }
            void printCmd() {
                DBG( "Time: " + gpstime + " Date: " + gpsdate + ", valid: " + valid +
                     ", lat: " + lat + ", lon: " + lon + ", alt: " + alt +
                     ", fix:" + String( fix ) + ", numSat: " + String( nosat ) );
            }

            void publishCmd() {
                String timestr = "";
                if ( gpstime != "" && gpsdate != "" ) {
                    timestr = "20" + gpsdate.substring( 4, 6 ) + "-" + gpsdate.substring( 2, 4 ) +
                              "-" + gpsdate.substring( 0, 2 ) + "T" + gpstime.substring( 0, 2 ) +
                              ":" + gpstime.substring( 2, 4 ) + ":" + gpstime.substring( 4, 6 ) +
                              "Z";
                    //time_t t;
                    //t=meisterwerk::util::msgtime::ISO2time_t(timestr);
                    //String back=meisterwerk::util::msgtime::time_t2ISO(t);
                    //DBG(timestr+"->"+back);
                }
                if ( valid == "A" ) {
                    String msg =
                        "{\"time\":\"" + timestr + "\",\"source\":\"GPS\",\"precision\":1000000}";
                    publish( entName + "/time", msg );
                } else {
                    if ( timestr != "" ) {
                        String msg =
                            "{\"time\":\"" + timestr + "\",\"source\":\"GPS-RTC\",\"precision\":0}";
                        publish( entName + "/time", msg );
                    } else {
                        String msg = "{\"time\":\"" + String( millis() ) + "\"}";
                    }
                }
                String gpsmsg = "{\"state\":\"";
                if ( valid == "A" )
                    gpsmsg += "ON";
                else
                    gpsmsg += "OFF";
                gpsmsg += "\",\"fix\":\"";
                if ( fix == 0 )
                    gpsmsg += "Fix not available or invalid";
                else if ( fix == 1 )
                    gpsmsg += "GSP SPS Mode, fix valid";
                else if ( fix == 2 )
                    gpsmsg += "Differential GPS, SPS Mode, fix valid";
                gpsmsg += "\",\"satellites\":" + String( nosat ) + "}";
                publish( entName + "/gps", gpsmsg );
            }
            void processCmd() {
                // DBG( cmd[0] );
                if ( cmd[0] == "$GPGGA" ) { // GGA —Global Positioning System Fixed Data
                    gpstime = cmd[1];
                    lat     = cmd[2] + cmd[3];
                    lon     = cmd[4] + cmd[5];
                    fix     = atoi( cmd[6].c_str() );
                    nosat   = atoi( cmd[7].c_str() );
                    alt     = cmd[9];

                } else if ( cmd[0] == "$GPRMC" ) {
                    gpsdate = cmd[9];
                    valid   = cmd[2];

                    // printCmd();
                    publishCmd();
                    resetDefaults();
                }
            }

            virtual void onLoop( unsigned long ticker ) override {
                if ( isOn ) {
                    while ( pser->available() > 0 ) {
                        char c = pser->read();
                        if ( c == 10 )
                            continue;
                        if ( c == 13 ) {
                            processCmd();
                            resetCmd();
                        } else {
                            if ( c == ',' ) {
                                ++icmd;
                                if ( icmd >= NMEA_MAX_CMDS ) {
                                    resetCmd();
                                }
                            } else {
                                cmd[icmd] += c;
                                if ( cmd[icmd].length() > 24 ) {
                                    resetCmd();
                                    DBG( "Illegal format received." );
                                }
                            }
                        }
                    }
                }
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                meisterwerk::core::entity::onReceive( origin, topic, msg );
            }

        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
