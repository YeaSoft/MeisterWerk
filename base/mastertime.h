
// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <ESP8266WiFi.h>
#include <Wire.h>

// dependencies
#include "../core/entity.h"
#include "../util/hextools.h"

namespace meisterwerk {
    namespace base {

        class mastertime : public meisterwerk::core::entity {
            public:
            enum TimeSourceType { NONE = 0, RTC = 1, GPS_RTC = 2, GPS = 5, NTP = 10 };
            enum TimeSourceType bestClock;
            unsigned long       timeStampBestClock;

            bool bSetup;

            mastertime( String name ) : meisterwerk::core::entity( name ) {
                bSetup = false;
            }

            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 50000 );
            }

            virtual void onRegister() override {
                bSetup = true;
                subscribe( "*/time" );
            }

            virtual void onLoop( unsigned long ticker ) override {
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                int    p  = topic.indexOf( "/" );
                String t1 = topic.substring( p + 1 );
                if ( t1 == "time" ) {
                    DynamicJsonBuffer jsonBuffer( 200 );
                    JsonObject &      root = jsonBuffer.parseObject( msg );
                    if ( !root.success() ) {
                        DBG( "Ntp: Invalid JSON received: " + msg );
                        return;
                    }
                    String state = root["time"];
                }
            }
        };
    } // namespace base
} // namespace meisterwerk
