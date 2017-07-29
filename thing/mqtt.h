// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

#include <functional>

#include <PubSubClient.h>

// dependencies
#include "../core/entity.h"
#include "../util/metronome.h"
#include "../util/msgtime.h"

namespace meisterwerk {
    namespace thing {

        class mqtt : public meisterwerk::core::entity {
            public:
            WiFiClient   wifiClient;
            PubSubClient mqttClient;
            bool         bMqInit = false;

            bool            isOn             = false;
            bool            netUp            = false;
            bool            mqttConnected    = false;
            bool            bCheckConnection = false;
            util::metronome mqttTicker;
            String          clientName;
            String          mqttServer;
            IPAddress       mqttserverIP;

            mqtt( String name )
                : meisterwerk::core::entity( name ), mqttClient( wifiClient ), mqttTicker( 5000L ), clientName{name} {
                mqttServer = "";
            }
            ~mqtt() {
                if ( isOn ) {
                    isOn = false;
                }
            }

            virtual void setup() override {
                bool ret = meisterwerk::core::entity::registerEntity( slice, core::scheduler::PRIORITY_NORMAL );
                DBG( "Init mqtt" );
                setLogLevel( T_LOGLEVEL::INFO );
                // subscribe( "net/network" );
                // subscribe( "net/services/mqttserver" );
                subscribe( "*" );
                publish( "net/network/get" );
                publish( "net/services/mqttserver/get" );
                isOn = true;
                return ret;
            }

            bool         bWarned = false;
            virtual void loop() override {
                if ( isOn ) {
                    if ( netUp && mqttServer != "" ) {
                        if ( mqttConnected ) {
                            mqttClient.loop();
                        }

                        if ( bCheckConnection || mqttTicker.beat() > 0 ) {
                            bCheckConnection = false;
                            if ( !mqttClient.connected() ) {
                                // Attempt to connect
                                if ( mqttClient.connect( clientName.c_str() ) ) {
                                    DBG( "MQTT connected to " + mqttServer );
                                    mqttConnected = true;
                                    mqttClient.subscribe( "mw/#" );
                                    bWarned = false;
                                } else {
                                    if ( !bWarned ) {
                                        DBG( "MQTT server connection failed." );
                                        bWarned = true;
                                    }
                                    mqttConnected = false;
                                }
                            }
                        }
                    }
                }
            }

            void onMqttReceive( char *ctopic, unsigned char *payload, unsigned int length ) {
                String msg = "";
                String topic;
                char   buf[24];
                sprintf( buf, "%010ld", millis() );
                DBG( String( buf ) + "MQR:" + String( ctopic ) );
                log( T_LOGLEVEL::INFO, String( msg ), String( ctopic ) );
                if ( strlen( ctopic ) > 3 )
                    topic = (char *)( &ctopic[3] ); // strip mw/   XXX: regex
                for ( int i = 0; i < length; i++ ) {
                    msg += (char)payload[i];
                }
                publish( topic, msg );
            }

            virtual void receive( const char *origin, const char *ctopic, const char *msg ) override {
                String topic( ctopic );
                if ( mqttConnected ) {
                    if ( topic.indexOf( "display/set" ) == -1 ) { // XXX: better filter config needed. (get/set)
                        unsigned int len = strlen( msg ) + 1;
                        if ( mqttClient.publish( ctopic, msg, len ) ) {
                            DBG( "MQTT publish: " + topic + " | " + String( msg ) );
                        } else {
                            DBG( "MQTT ERROR len=" + String( len ) + ", not published: " + topic + " | " +
                                 String( msg ) );
                            if ( len > 128 ) {
                                DBG(
                                    "FATAL ERROR: you need to re-compile the PubSubClient library and increase #define "
                                    "MQTT_MAX_PACKET_SIZE." );
                            }
                        }
                    }
                } else
                    DBG( "MQTT can't publish, MQTT down: " + topic );

                DynamicJsonBuffer jsonBuffer( 200 );
                JsonObject &      root = jsonBuffer.parseObject( msg );
                if ( !root.success() ) {
                    DBG( "mqtt: Invalid JSON received: " + String( msg ) );
                    return;
                }
                if ( topic == "net/services/mqttserver" ) {
                    if ( !bMqInit ) {
                        mqttServer       = root["server"].as<char *>();
                        bCheckConnection = true;
                        DBG( "mqtt: received server address: " + mqttServer );
                        mqttClient.setServer( mqttServer.c_str(), 1883 );
                        // give a c++11 lambda as callback for incoming mqtt messages:
                        std::function<void( char *, unsigned char *, unsigned int )> f =
                            [=]( char *t, unsigned char *m, unsigned int l ) { this->onMqttReceive( t, m, l ); };
                        mqttClient.setCallback( f );
                        bMqInit = true;
                        // mqttClient.setCallback( onMqttReceive );
                    }
                }
                if ( topic == "net/network" ) {
                    String state = root["state"];
                    if ( state == "connected" ) {
                        if ( !netUp ) {
                            netUp            = true;
                            bCheckConnection = true;
                        }
                    } else {
                        netUp = false;
                    }
                }
            }
        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
