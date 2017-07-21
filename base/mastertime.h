
// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <ESP8266WiFi.h>

// dependencies
#include "../core/entity.h"
#include "../util/msgtime.h"

namespace meisterwerk {
    namespace base {
        enum TimeSourceType { NONE            = 0, RTC, GPS_RTC, DCF77, GPS, NTP };
        static const String TimeSourceNames[] = {"None", "RTC", "GPS-RTC", "DCF-77", "GPS", "NTP"};
        typedef struct {
            TimeSourceType type;
            String         name;
            unsigned long  lastActive;
        } T_TIMESOURCE;

        class mastertime : public meisterwerk::core::entity {
            public:
            TimeSourceType bestClock;
            unsigned long  timeStampBestClock;

            bool bSetup;

            mastertime( String name ) : meisterwerk::core::entity( name ) {
                bSetup    = false;
                bestClock = TimeSourceType::NONE;
            }

            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity(
                    10000, core::scheduler::PRIORITY_TIMECRITICAL );
            }

            virtual void onRegister() override {
                bSetup = true;
                subscribe( "*/time" );
                publish( "*/time/get" );
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
                    String         isoTime    = root["time"];
                    String         timeSource = root["timesource"];
                    time_t         t          = util::msgtime::ISO2time_t( isoTime );
                    TimeSourceType cur        = TimeSourceType::NONE;
                    for ( int i = 0; i < sizeof( TimeSourceNames ) / sizeof( String * ); i++ ) {
                        if ( TimeSourceNames[i] == timeSource )
                            cur = (TimeSourceType)i;
                    }
                    if ( cur > bestClock ) {
                        bestClock = cur;
                        setTime( t );
                        DBG( "System-time set by clock of type: " + timeSource );
                        publish( "mastertime/time/set", msg );
                    }
                    // XXX: register clock types
                    // XXX: periodic update by best clock
                    // XXX: check timeouts for registered clocks.
                }
            }
        };
    } // namespace base
} // namespace meisterwerk
