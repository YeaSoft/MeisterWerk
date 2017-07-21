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
            bool    bPublishTime   = false;
            bool    bPublishGps    = false;

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
                subscribe( entName + "gps/get" );
                subscribe( entName + "time/get" );
                // subscribe( entName + "/gps/set" );
                // subscribe( entName + "/time/set" );
                isOn = true;
            }

            int l = 0;
#define NMEA_MAX_CMDS 32
            String cmd[NMEA_MAX_CMDS];
            int    icmd    = 0;
            String gpstime = "", gpsdate = "", lat = "", lath = "", lon = "", lonh = "", alt = "",
                   valid = "";
            int nosat    = 0;
            int fix      = 0;

            void resetCmd() {
                for ( int i = 0; i < NMEA_MAX_CMDS; i++ )
                    cmd[i]  = "";
                icmd        = 0;
            }

            void resetDefaults() {
                icmd    = 0;
                gpstime = "";
                gpsdate = "";
                lat     = "";
                lon     = "";
                lath    = "";
                lonh    = "";
                alt     = "";
                valid   = "";
                nosat   = 0;
                fix     = 0;
            }
            void printCmd() {
                DBG( "Time: " + gpstime + " Date: " + gpsdate + ", valid: " + valid +
                     ", lat: " + lat + lath + ", lon: " + lon + lonh + ", alt: " + alt +
                     ", fix:" + String( fix ) + ", numSat: " + String( nosat ) );
            }

            String parseTimeToIsoJsonElement( int fix ) {
                static int afix    = -1;
                String     timestr = "";
                String     msg     = "";
                if ( gpstime != "" && gpsdate != "" ) {
                    timestr = "20" + gpsdate.substring( 4, 6 ) + "-" + gpsdate.substring( 2, 4 ) +
                              "-" + gpsdate.substring( 0, 2 ) + "T" + gpstime.substring( 0, 2 ) +
                              ":" + gpstime.substring( 2, 4 ) + ":" + gpstime.substring( 4, 6 ) +
                              "Z";

                    if ( gpstime.substring( 4, 6 ) == "00" ) {
                        bPublishTime = true;
                    }
                    if ( fix != afix ) {
                        afix         = fix;
                        bPublishTime = true;
                    }

                    // time_t t;
                    // t=util::msgtime::ISO2time_t(timestr);
                    // String back=util::msgtime::time_t2ISO(t);
                    // DBG(timestr+"->"+back);
                }
                if ( valid == "A" ) {
                    msg = "\"time\":\"" + timestr +
                          "\",\"timesource\":\"GPS\",\"timeprecision\":1000000";
                } else {
                    if ( timestr != "" ) {
                        msg = "\"time\":\"" + timestr +
                              "\",\"timesource\":\"GPS-RTC\",\"timeprecision\":0";
                    } else {
                        msg = "\"time\":\"" + String( millis() ) + "\"";
                    }
                }
                return msg;
            }

            bool sigdelta( double a, double b, double eps ) {
                double dx = a - b;
                if ( dx < 0.0 )
                    dx = dx * ( -1.0 );
                if ( dx > eps )
                    return true;
                else
                    return false;
            }
            bool detectGpsChange( double flon, double flat, int nosat, int fix, double alt ) {
                static int    anosat = -1;
                static int    afix   = -1;
                static double aflon  = ( -1000.0 );
                static double aflat  = ( -1000.0 );
                static double aalt   = ( -1000.0 );
                bool          ret    = false;
                if ( nosat != anosat ) {
                    if ( nosat < 4 || abs( nosat - anosat ) > 1 ) {
                        anosat = nosat;
                        ret    = true;
                    }
                }
                if ( fix != afix ) {
                    afix = fix;
                    ret  = true;
                }
                if ( sigdelta( flon, aflon, 0.001 ) ) {
                    aflon = flon;
                    ret   = true;
                }
                if ( sigdelta( flat, aflat, 0.001 ) ) {
                    aflat = flat;
                    ret   = true;
                }
                if ( sigdelta( alt, aalt, 5.0 ) ) {
                    aalt = alt;
                    ret  = true;
                }

                return ret;
            }

            String parseGpsDataToJsonElement() {
                double flon, flat;
                String gpsmsg = "\"state\":\"";
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
                gpsmsg += "\",\"satellites\":" + String( nosat );
                if ( valid == "A" ) {
                    gpsmsg += ",\"altitude\":" + alt;
                    flon       = atof( lon.c_str() );
                    double deg = (double)( (int)( flon / 100 ) );
                    double min = flon - 100.0 * deg;
                    flon       = deg + min / 60.0;
                    flat       = atof( lat.c_str() );
                    deg        = (double)( (int)( flat / 100 ) );
                    min        = flat - 100.0 * deg;
                    flat       = deg + min / 60.0;
                    char bufp[32];
                    dtostrf( flat, 10, 6, bufp );
                    gpsmsg += ",\"lat\":" + String( bufp );
                    gpsmsg += ",\"lath\":\"" + lath + "\"";
                    // DBG( lon + "," + String( bufp ) );
                    dtostrf( flon, 10, 6, bufp );
                    gpsmsg += ",\"lon\":" + String( bufp );
                    gpsmsg += ",\"lonh\":\"" + lonh + "\"";
                    // DBG( lat + "," + String( bufp ) );
                    if ( detectGpsChange( flon, flat, nosat, fix, atof( alt.c_str() ) ) )
                        bPublishGps = true;
                }
                return gpsmsg;
            }

            void publishCmd() {
                String timestr = parseTimeToIsoJsonElement( fix );
                String gpsmsg  = parseGpsDataToJsonElement();

                if ( bPublishGps ) {
                    publish( entName + "/gps", "{" + timestr + "," + gpsmsg + "}" );
                    bPublishGps = false;
                }
                if ( bPublishTime ) {
                    publish( entName + "/time", "{" + timestr + "}" );
                    bPublishTime = false;
                }
            }
            void processCmd() {
                // DBG( cmd[0] );
                if ( cmd[0] == "$GPGGA" ) { // GGA —Global Positioning System Fixed Data
                    gpstime = cmd[1];
                    lat     = cmd[2];
                    lath    = cmd[3];
                    lon     = cmd[4];
                    lonh    = cmd[5];
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
                                    DBG( "Illegal format received: " + cmd[icmd] );
                                    resetCmd();
                                }
                            }
                        }
                    }
                }
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                meisterwerk::core::entity::onReceive( origin, topic, msg );
                DBG( "GpsReceive:" + topic + "," + msg );
                if ( topic == entName + "/time/get" || topic == "*/time/get" ) {
                    bPublishTime = true;
                }
                if ( topic == entName + "/gps/get" ) {
                    bPublishGps = true;
                }
            }

        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
