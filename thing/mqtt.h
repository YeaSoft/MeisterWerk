// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

#include <functional>
// hardware dependencies
#define MQTT_MAX_PACKET_SIZE 1024
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

            bool            isOn             = false;
            bool            netUp            = false;
            bool            mqttConnected    = false;
            bool            bCheckConnection = false;
            util::metronome mqttTicker;
            String          clientName;
            String          mqttServer;
            IPAddress       mqttserverIP;

            mqtt( String name )
                : meisterwerk::core::entity( name ), mqttClient( wifiClient ),
                  mqttTicker( 5000L ), clientName{name} {
                mqttServer = "";
            }
            ~mqtt() {
                if ( isOn ) {
                    isOn = false;
                }
            }

            bool registerEntity() {
                bool ret = meisterwerk::core::entity::registerEntity(
                    50000, core::scheduler::PRIORITY_NORMAL );
                DBG( "Init mqtt" );
                subscribe( "net/network" );
                subscribe( "net/services/mqttserver" );
                subscribe( "*" );
                publish( "net/network/get" );
                publish( "net/services/mqttserver/get" );
                isOn = true;
            }

            bool         bWarned = false;
            virtual void onLoop( unsigned long ticker ) override {
                if ( isOn ) {
                    if ( netUp && mqttServer != "" ) {
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
                            if ( mqttConnected ) {
                                mqttClient.loop();
                            }
                        }
                    }
                }
            }

            void onMqttReceive( char *ctopic, unsigned char *payload, unsigned int length ) {
                String msg = "";
                String topic;
                if ( strlen( ctopic ) > 3 )
                    topic = (char *)( &ctopic[3] ); // strip mw/
                for ( int i = 0; i < length; i++ ) {
                    msg += (char)payload[i];
                }
                publish( topic, msg );
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                // meisterwerk::core::entity::onReceive( origin, topic, msg );

                if ( mqttConnected )
                    mqttClient.publish( topic.c_str(), msg.c_str() );

                if ( topic == "net/services/mqttserver" ) {
                    DynamicJsonBuffer jsonBuffer( 200 );
                    JsonObject &      root = jsonBuffer.parseObject( msg );
                    if ( !root.success() ) {
                        DBG( "mqtt: Invalid JSON received: " + msg );
                        return;
                    }
                    mqttServer       = root["server"].as<char *>();
                    bCheckConnection = true;
                    DBG( "mqtt: received server address: " + mqttServer );
                    mqttClient.setServer( mqttServer.c_str(), 1883 );
                    // give a c++11 lambda as callback for incoming mqtt messages:
                    std::function<void( char *, unsigned char *, unsigned int )> f =
                        [=]( char *t, unsigned char *m, unsigned int l ) {
                            this->onMqttReceive( t, m, l );
                        };
                    mqttClient.setCallback( f );
                    // mqttClient.setCallback( onMqttReceive );
                }
                if ( topic == "net/network" ) {
                    DynamicJsonBuffer jsonBuffer( 200 );
                    JsonObject &      root = jsonBuffer.parseObject( msg );
                    if ( !root.success() ) {
                        DBG( "mqtt: Invalid JSON received: " + msg );
                        return;
                    }
                    String state = root["state"];
                    if ( state == "connected" ) {
                        netUp            = true;
                        bCheckConnection = true;
                    } else {
                        netUp = false;
                    }
                }
            }
        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
