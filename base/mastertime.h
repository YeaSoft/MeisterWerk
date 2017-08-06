
// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <ESP8266WiFi.h>
#include <map>

// dependencies
#include "../core/entity.h"
#include "../util/msgtime.h"

namespace meisterwerk {
    namespace base {
        enum TimeType { NONE = 0, RTC, GPS_RTC, HP_RTC, NTP, DCF77, GPS };
        std::map<String, TimeType> TimeName2Type = {{"None", TimeType::NONE},       {"RTC", TimeType::RTC},
                                                    {"GPS-RTC", TimeType::GPS_RTC}, {"HP-RTC", TimeType::HP_RTC},
                                                    {"NTP", TimeType::NTP},         {"DCF-77", TimeType::DCF77},
                                                    {"GPS", TimeType::GPS}};
        typedef struct {
            TimeType      type;
            String        typeName;
            unsigned long lastActive;
        } T_TIMESOURCE;

        class mastertime : public meisterwerk::core::entity {
            public:
            TimeType      bestClock;
            TimeType      oldBestClock;
            unsigned long timeStampBestClock;
            unsigned long clockRefreshTimeout   = 3600; // Don't wait for a clock that's dead for one hour of more
            unsigned long clockRefreshIntervall = 900;  // Send clock updates after each 15min.

            std::map<String, T_TIMESOURCE *> clocks;
            bool                             bSetup;

            mastertime( String name ) : meisterwerk::core::entity( name, 50000 ) {
                bSetup       = false;
                bestClock    = TimeType::NONE;
                oldBestClock = TimeType::NONE;
            }
            ~mastertime() {
                for ( auto p : clocks ) {
                    free( p.second );
                }
            }

            virtual void setup() override {
                bSetup = true;
                subscribe( "+/time" );
                publish( "time/get" );
            }

            virtual void loop() override {
            }

            virtual void receive( const char *origin, const char *ctopic, const char *msg ) override {
                String topic( ctopic );
                int    p = topic.indexOf( "/" );
                time_t tlast;
                String t1 = topic.substring( p + 1 );
                if ( t1 == "time" ) {
                    DBG( "Mastertime: " + String( msg ) );
                    DynamicJsonBuffer jsonBuffer( 200 );
                    JsonObject &      root = jsonBuffer.parseObject( msg );
                    if ( !root.success() ) {
                        DBG( "Ntp: Invalid JSON received: " + String( msg ) );
                        return;
                    }
                    String isoTime = root["time"];
                    DBG( "Mastertime: " + isoTime );
                    String   timeSource = root["timesource"];
                    time_t   t          = util::msgtime::ISO2time_t( isoTime );
                    TimeType cur        = TimeType::NONE;
                    if ( TimeName2Type.find( timeSource ) == TimeName2Type.end() ) {
                        DBG( "MasterTime: Invalid TimeSource type received!" );
                    } else {
                        cur = TimeName2Type[timeSource];
                    }
                    String clockname = String( origin ) + "-" + timeSource;
                    if ( clocks.find( clockname ) == clocks.end() ) {
                        DBG( "New clockType " + timeSource + " at: " + origin );
                        clocks[clockname] = (T_TIMESOURCE *)malloc( sizeof( T_TIMESOURCE ) );
                        memset( clocks[clockname], 0, sizeof( T_TIMESOURCE ) );
                    }
                    clocks[clockname]->typeName   = timeSource;
                    tlast                         = clocks[clockname]->lastActive;
                    clocks[clockname]->lastActive = t;
                    clocks[clockname]->type       = cur;

                    bestClock = TimeType::NONE;
                    for ( auto p : clocks ) {
                        if ( now() - p.second->lastActive < clockRefreshTimeout || p.second->type == cur ) {
                            if ( p.second->type > bestClock )
                                bestClock = p.second->type;
                        }
                    }
                    if ( bestClock == cur ) {
                        if ( bestClock != oldBestClock || now() - tlast > clockRefreshIntervall ) {
                            oldBestClock = bestClock;
                            setTime( t );
                            DBG( "System-time set by clock of type: " + timeSource );
                            publish( "mastertime/time/set", msg );
                        }
                    }
                }
            }
        };
    } // namespace base
} // namespace meisterwerk
