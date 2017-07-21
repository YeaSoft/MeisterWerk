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
#include <WiFiUdp.h>

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
        class Ntp : public meisterwerk::core::entity {
            public:
            enum Udpstate { IDLE, PACKETSENT };
            Udpstate        ntpstate;
            bool            isOn     = false;
            bool            netUp    = false;
            bool            bGetTime = false;
            util::metronome ntpTicker;
            unsigned long   packettimestamp;
            unsigned long   ntpTimeout = 3e00; // ms timeout for send/receive NTP packets.
            String          ntpServer;
            bool            ipNtpInit = false;
            IPAddress       timeServerIP; // time.nist.gov NTP server address
                                          // const char* ntpServerName = "time.nist.gov";
#define NTP_PACKET_SIZE 48                // NTP time stamp is in the first 48 bytes of the message
            byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming and outgoing packets

            // A UDP instance to let us send and receive packets over UDP
            WiFiUDP udp;

            unsigned int localPort = 2390; // local port to listen for UDP packets

            Ntp( String name ) : meisterwerk::core::entity( name ), ntpTicker( 300000L ) {
                ntpstate  = Udpstate::IDLE;
                ntpServer = "";
            }
            ~Ntp() {
                if ( isOn ) {
                    isOn = false;
                }
            }

            bool registerEntity() {
                // 5sec sensor checks
                bool ret = meisterwerk::core::entity::registerEntity(
                    50000, core::scheduler::PRIORITY_TIMECRITICAL );
                DBG( "init ntp." );
                subscribe( "net/network" );
                subscribe( entName + "/time/get" );
                subscribe( "net/services/timeserver" );
                publish( "net/network/get" );
                publish( "net/services/timeserver/get" );
                isOn = true;
            }

            // send an NTP request to the time server at the given address
            unsigned long sendNTPpacket( IPAddress &address ) {
                // set all bytes in the buffer to 0
                memset( packetBuffer, 0, NTP_PACKET_SIZE );
                // Initialize values needed to form NTP request
                // (see URL above for details on the packets)
                packetBuffer[0] = 0b11100011; // LI, Version, Mode
                packetBuffer[1] = 0;          // Stratum, or type of clock
                packetBuffer[2] = 6;          // Polling Interval
                packetBuffer[3] = 0xEC;       // Peer Clock Precision
                // 8 bytes of zero for Root Delay & Root Dispersion
                packetBuffer[12] = 49;
                packetBuffer[13] = 0x4E;
                packetBuffer[14] = 49;
                packetBuffer[15] = 52;

                // all NTP fields have been given values, now
                // you can send a packet requesting a timestamp:
                udp.beginPacket( address, 123 ); // NTP requests are to port 123
                udp.write( packetBuffer, NTP_PACKET_SIZE );
                udp.endPacket();
            }

            bool getNtpTime() {
                if ( !ipNtpInit ) {
                    DBG( "NTP: resolving timeServer: " + ntpServer );
                    WiFi.hostByName( ntpServer.c_str(), timeServerIP );
                    ipNtpInit = true;
                }

                sendNTPpacket( timeServerIP ); // send an NTP packet to a time server
                ntpstate        = Udpstate::PACKETSENT;
                packettimestamp = millis();
                return true;
            }

            bool parseNtpTime() {
                int cb = udp.parsePacket();
                if ( cb ) {
                    // display.print("Rcv",6,10);
                    // We've received a packet, read the data from it
                    udp.read( packetBuffer, NTP_PACKET_SIZE ); // read the packet into the buffer

                    // the timestamp starts at byte 40 of the received packet and is four bytes,
                    // or two words, long. First, esxtract the two words:

                    unsigned long highWord = word( packetBuffer[40], packetBuffer[41] );
                    unsigned long lowWord  = word( packetBuffer[42], packetBuffer[43] );
                    // combine the four bytes (two words) into a long integer
                    // this is NTP time (seconds since Jan 1 1900):
                    unsigned long secsSince1900 = highWord << 16 | lowWord;

                    // now convert NTP time into everyday time:
                    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
                    const unsigned long seventyYears = 2208988800UL;
                    // subtract seventy years:
                    unsigned long epoch = secsSince1900 - seventyYears;

                    String isoTime = util::msgtime::time_t2ISO( epoch );
                    String msg     = "{\"time\":\"" + isoTime +
                                 "\",\"timesource\":\"NTP\",\"timeprecision\":10000}";
                    publish( entName + "/time", msg );
                    return true;
                } else {
                    return false;
                }
            }

            virtual void onLoop( unsigned long ticker ) override {
                if ( isOn ) {
                    if ( netUp ) {
                        switch ( ntpstate ) {
                        case Udpstate::IDLE:
                            if ( bGetTime || ntpTicker.beat() > 0 ) {
                                bGetTime = false;
                                if ( ntpServer != "" )
                                    getNtpTime();
                            }
                            break;
                        case Udpstate::PACKETSENT:
                            if ( parseNtpTime() ) {
                                ntpstate = Udpstate::IDLE;
                            } else {
                                if ( util::timebudget::delta( packettimestamp, millis() ) >
                                     ntpTimeout ) {
                                    DBG( "NTP timeout receiving from server: " + ntpServer );
                                    ntpstate = Udpstate::IDLE;
                                }
                            }
                            break;
                        default:
                            DBG( "NTP: Illegal Udpstate!" );
                            break;
                        }
                    }
                }
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                // meisterwerk::core::entity::onReceive( origin, topic, msg );
                DBG( "Ntp:" + topic + "," + msg );
                if ( topic == entName + "/time/get" || topic == "*/time/get" ) {
                    if ( !netUp ) {
                        DBG( "NTP: Cannot get time, net down." );
                    } else {
                        bGetTime = true;
                    }
                }
                if ( topic == "net/services/timeserver" ) {
                    DynamicJsonBuffer jsonBuffer( 200 );
                    JsonObject &      root = jsonBuffer.parseObject( msg );
                    if ( !root.success() ) {
                        DBG( "Ntp: Invalid JSON received: " + msg );
                        return;
                    }
                    ntpServer = root["server"].as<char *>();
                    DBG( "NTP: received server address: " + ntpServer );
                    if ( netUp && ntpServer != "" ) {
                        getNtpTime();
                    }
                }
                if ( topic == "net/network" ) {
                    DynamicJsonBuffer jsonBuffer( 200 );
                    JsonObject &      root = jsonBuffer.parseObject( msg );
                    if ( !root.success() ) {
                        DBG( "Ntp: Invalid JSON received: " + msg );
                        return;
                    }
                    String state = root["state"];
                    if ( state == "connected" ) {
                        netUp = true;
                        udp.begin( localPort );
                        if ( ntpServer != "" ) {
                            getNtpTime();
                        }
                    } else
                        netUp = false;
                }
            }

        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
