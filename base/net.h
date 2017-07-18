
// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <ESP8266WiFi.h>
#include <FS.h>

// dependencies
#include "../core/entity.h"
#include "../util/hextools.h"

namespace meisterwerk {
    namespace base {

        class net : public meisterwerk::core::entity {
            public:
            enum Netstate { NOTCONFIGURED, CONNECTINGAP, CONNECTED };

            bool     bSetup;
            Netstate state;
            long     contime;
            long     conto = 15000;
            String   SSID;
            String   password;
            String   lhostname;
            String   ipaddress;

            net( String name ) : meisterwerk::core::entity( name ) {
                bSetup = false;
            }

            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 50000 );
            }

            virtual void onRegister() override {
                bSetup = true;
                state  = Netstate::NOTCONFIGURED;
                SPIFFS.begin();
                File f = SPIFFS.open( "/net.json", "r" );
                if ( !f ) {
                    DBG( "SPIFFS needs to contain a net.json file!" );
                } else {
                    String jsonstr = "";
                    while ( f.available() ) {
                        // Lets read line by line from the file
                        String lin = f.readStringUntil( '\n' );
                        jsonstr    = jsonstr + lin;
                    }
                    DynamicJsonBuffer jsonBuffer( 200 );
                    JsonObject &      root = jsonBuffer.parseObject( jsonstr );
                    if ( !root.success() ) {
                        DBG( "Invalid JSON received, check SPIFFS file net.json!" );
                    } else {
                        SSID     = root["SSID"].as<char*>();
                        password = root["password"].as<char*>();
                        lhostname = root["hostname"].as<char*>();
                        DBG( "Connecting to: " + SSID );
                        WiFi.mode( WIFI_STA );
                        WiFi.begin( SSID.c_str(), password.c_str() );
                        if ( lhostname != "" )
                            WiFi.hostname( lhostname.c_str() );
                        state   = Netstate::CONNECTINGAP;
                        contime = millis();
                    }
                }
                if ( state == Netstate::CONNECTINGAP )
                    publish( "net/connecting", "{\"SSID\":\"" + SSID + "\"}" );
                else
                    publish( "net/notconfigured" );
            }

            virtual void onLoop( unsigned long ticker ) override {
                switch ( state ) {
                case Netstate::CONNECTINGAP:
                    if ( WiFi.status() == WL_CONNECTED ) {
                        state        = Netstate::CONNECTED;
                        IPAddress ip = WiFi.localIP();
                        ipaddress  = String( ip[0] ) + '.' + String( ip[1] ) + '.' +
                                      String( ip[2] ) + '.' + String( ip[3] );
                        publish( "net/connected", "{\"SSID\":\"" + SSID + "\",\"hostname\":\""+lhostname+"\",\"IP\":\""+ipaddress+"\"}" );
                    }
                    if ( util::timebudget::delta( contime, millis() ) > conto ) {
                        DBG( "Timeout connecting to: " + SSID );
                        state = Netstate::NOTCONFIGURED;
                        publish( "net/notconfigured" );
                    }
                    break;
                case Netstate::CONNECTED:
                // XXX: verify connection state...
                break;
                default:
                    break;
                }
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
            }
        };
    } // namespace base
} // namespace meisterwerk
