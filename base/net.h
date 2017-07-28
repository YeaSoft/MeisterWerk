
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
#include <map>

// dependencies
#include "../core/entity.h"
#include "../util/hextools.h"
#include "../util/metronome.h"
#include "../util/sensorprocessor.h"

namespace meisterwerk {
    namespace base {

        class net : public meisterwerk::core::entity {
            public:
            enum Netstate { NOTDEFINED, NOTCONFIGURED, CONNECTINGAP, CONNECTED };
            enum Netmode { AP, STATION };

            bool                     bSetup;
            Netstate                 state;
            Netstate                 oldstate;
            Netmode                  mode;
            long                     contime;
            long                     conto = 15000;
            String                   SSID;
            String                   password;
            String                   lhostname;
            String                   ipaddress;
            util::metronome          tick1sec;
            util::metronome          tick10sec;
            util::sensorprocessor    rssival;
            std::map<String, String> netservices;
            String                   macAddress;

            net( String name )
                : meisterwerk::core::entity( name ), tick1sec( 1000L ), tick10sec( 10000L ), rssival( 5, 900, 2.0 ) {
                bSetup = false;
            }

            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 50000 );
            }

            void publishNetwork() {
                String json;
                if ( mode == Netmode::AP ) {
                    json = "{\"mode\":\"ap\",";
                } else if ( mode == Netmode::STATION ) {
                    json = "{\"mode\":\"station\",";
                } else {
                    json = "{\"mode\":\"undefined\",";
                }
                json += "\"mac\":\"" + macAddress + "\",";
                switch ( state ) {
                case Netstate::NOTCONFIGURED:
                    json += "\"state\":\"notconfigured\"}";
                    break;
                case Netstate::CONNECTINGAP:
                    json += "\"state\":\"connectingap\",\"SSID\":\"" + SSID + "\"}";
                    break;
                case Netstate::CONNECTED:
                    json += "\"state\":\"connected\",\"SSID\":\"" + SSID + "\",\"hostname\":\"" + lhostname +
                            "\",\"ip\":\"" + ipaddress + "\"}";
                    break;
                default:
                    json += "\"state\":\"undefined\"}";
                    break;
                }
                publish( "net/network", json );
                Log( loglevel::INFO, json );
                if ( state == Netstate::CONNECTED )
                    publishServices();
            }

            bool readNetConfig() {
                SPIFFS.begin();
                File f = SPIFFS.open( "/net.json", "r" );
                if ( !f ) {
                    DBG( "SPIFFS needs to contain a net.json file!" );
                    return false;
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
                        return false;
                    } else {
                        SSID             = root["SSID"].as<char *>();
                        password         = root["password"].as<char *>();
                        lhostname        = root["hostname"].as<char *>();
                        JsonArray &servs = root["services"];
                        for ( int i = 0; i < servs.size(); i++ ) {
                            JsonObject &srv = servs[i];
                            for ( auto obj : srv ) {
                                netservices[obj.key] = obj.value.as<char *>();
                            }
                        }
                    }

                    // for ( auto s : netservices ) {
                    //    DBG( "***" + s.first + "->" + s.second );
                    //}
                    return true;
                }
            }

            void connectAP() {
                DBG( "Connecting to: " + SSID );
                WiFi.mode( WIFI_STA );
                WiFi.begin( SSID.c_str(), password.c_str() );
                macAddress = WiFi.macAddress();

                if ( lhostname != "" )
                    WiFi.hostname( lhostname.c_str() );
                state   = Netstate::CONNECTINGAP;
                contime = millis();
            }

            virtual void onRegister() override {
                bSetup   = true;
                oldstate = Netstate::NOTDEFINED;
                state    = Netstate::NOTCONFIGURED;
                mode     = Netmode::AP;
                if ( readNetConfig() ) {
                    connectAP();
                }
                subscribe( "net/network/get" );
                subscribe( "net/network/set" );
                subscribe( "net/networks/get" );
            }

            String strEncryptionType( int thisType ) {
                // read the encryption type and print out the name:
                switch ( thisType ) {
                case ENC_TYPE_WEP:
                    return "WEP";
                    break;
                case ENC_TYPE_TKIP:
                    return "WPA";
                    break;
                case ENC_TYPE_CCMP:
                    return "WPA2";
                    break;
                case ENC_TYPE_NONE:
                    return "None";
                    break;
                case ENC_TYPE_AUTO:
                    return "Auto";
                    break;
                default:
                    return "unknown";
                    break;
                }
            }
            void publishNetworks() {
                int numSsid = WiFi.scanNetworks();
                if ( numSsid == -1 ) {
                    publish( "net/networks", "{}" ); // "{\"state\":\"error\"}");
                    return;
                }
                String netlist = "{";
                for ( int thisNet = 0; thisNet < numSsid; thisNet++ ) {
                    if ( thisNet > 0 )
                        netlist += ",";
                    netlist += "\"" + WiFi.SSID( thisNet ) + "\":{\"rssi\":" + String( WiFi.RSSI( thisNet ) ) +
                               ",\"enc\":\"" + strEncryptionType( WiFi.encryptionType( thisNet ) ) + "\"}";
                }
                netlist += "}";
                publish( "net/networks", netlist );
            }

            void publishServices() {
                for ( auto s : netservices ) {
                    publish( "net/services/" + s.first, "{\"server\":\"" + s.second + "\"}" );
                }
            }
            virtual void onLoop( unsigned long ticker ) override {
                switch ( state ) {
                case Netstate::NOTCONFIGURED:
                    if ( tick10sec.beat() > 0 ) {
                        publishNetworks();
                    }
                    break;
                case Netstate::CONNECTINGAP:
                    if ( WiFi.status() == WL_CONNECTED ) {
                        state        = Netstate::CONNECTED;
                        IPAddress ip = WiFi.localIP();
                        ipaddress =
                            String( ip[0] ) + '.' + String( ip[1] ) + '.' + String( ip[2] ) + '.' + String( ip[3] );
                    }
                    if ( util::timebudget::delta( contime, millis() ) > conto ) {
                        DBG( "Timeout connecting to: " + SSID );
                        state = Netstate::NOTCONFIGURED;
                    }
                    break;
                case Netstate::CONNECTED:
                    if ( tick1sec.beat() ) {
                        if ( WiFi.status() == WL_CONNECTED ) {
                            long rssi = WiFi.RSSI();
                            if ( rssival.filter( &rssi ) ) {
                                publish( "net/rssi", "{\"rssi\":" + String( rssi ) + "}" );
                            }
                        } else {
                            state = Netstate::NOTCONFIGURED;
                        }
                    }
                    break;
                default:
                    break;
                }
                if ( state != oldstate ) {
                    oldstate = state;
                    publishNetwork();
                }
            }

            virtual void onReceive( const char *origin, const char *ctopic, const char *msg ) override {
                String topic( ctopic );
                Log( loglevel::INFO, String( msg ), topic );
                if ( topic == "net/network/get" ) {
                    publishNetwork();
                } else if ( topic == "net/networks/get" ) {
                    publishNetworks();
                } else if ( topic == "net/network/set" ) {
                    DBG( "network/set not yet implemented!" );
                }
                for ( auto s : netservices ) {
                    if ( topic == "net/services/" + s.first + "/get" ) {
                        publish( "net/services/" + s.first, "{\"server\":\"" + s.second + "\"}" );
                    }
                }
            }
        };
    } // namespace base
} // namespace meisterwerk
