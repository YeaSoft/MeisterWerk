// dumper.h - A helper entity for dumping system info
//
// This is the declaration of a class performing
// information dumps emitted through the serial
// interface.
// The class can be configured to dump information
// at regular intervals or only on request.
// The class works only if #define DEBUG

#pragma once

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../core/entity.h"
#include "../util/metronome.h"

namespace meisterwerk {
    namespace util {

        class dumper : public meisterwerk::core::entity {
            public:
#ifdef _MW_DEBUG
            meisterwerk::util::metronome autodump;
            String                       debugButton;

            dumper( String name = "dmp", unsigned long autodump = 0, String debugButton = "dbg" )
                : meisterwerk::core::entity( name ), autodump{autodump}, debugButton{debugButton} {
            }

            bool registerEntity( unsigned long             minMicroSecs = 250000,
                                 meisterwerk::core::T_PRIO priority     = meisterwerk::core::PRIORITY_NORMAL ) {
                // default precision for autodump rate is 250ms
                return meisterwerk::core::entity::registerEntity( minMicroSecs, priority );
            }

            void setup() override {
                meisterwerk::core::entity::setup();
                // explicit commands
                subscribe( "*/dump" );
                subscribe( "*/sysinfo" );
                subscribe( "*/taskinfo" );
                // events from debug Button
                subscribe( debugButton + "/short" );
                subscribe( debugButton + "/long" );
                subscribe( debugButton + "/extralong" );
                // dump system info on start
                dumpSystemInfo();
            }

            virtual void loop() override {
                if ( autodump.beat() ) {
                    dumpRuntimeInfo();
                }
            }

            virtual void receive( const char *origin, const char *topic, const char *msg ) override {
                String t( topic );
                // process my own subscriptions
                if ( t == debugButton + "/short" ) {
                    dumpRuntimeInfo();
                } else if ( t == debugButton + "/long" ) {
                    dumpSystemInfo();
                } else if ( t == debugButton + "/extralong" ) {
                    dumpTaskInfo();
                }
            }

            void dumpSystemInfo() {
                String                     pre   = "dumper(" + entName + ") ";
                const __FlashStringHelper *bytes = F( " bytes" );
                DBG( "" );
                DBG( pre + F( "System Information:" ) );
                DBG( pre + F( "-------------------" ) );
                DBG( pre + F( "Chip ID: " ) + ESP.getChipId() );
                DBG( pre + F( "Core Verion: " ) + ESP.getCoreVersion() );
                DBG( pre + F( "SDK Verion: " ) + ESP.getSdkVersion() );
                DBG( pre + F( "CPU Frequency: " ) + ESP.getCpuFreqMHz() + " MHz" );
                DBG( pre + F( "Program Size: " ) + ESP.getSketchSize() + bytes );
                DBG( pre + F( "Program Free: " ) + ESP.getFreeSketchSpace() + bytes );
                DBG( pre + F( "Flash Chip ID: " ) + ESP.getFlashChipId() );
                DBG( pre + F( "Flash Chip Size: " ) + ESP.getFlashChipSize() + bytes );
                DBG( pre + F( "Flash Chip Real Size: " ) + ESP.getFlashChipRealSize() + bytes );
                DBG( pre + F( "Flash Chip Speed: " ) + ESP.getFlashChipSpeed() + " hz" );
                DBG( pre + F( "Last Reset Reason: " ) + ESP.getResetReason() );
            }

            void dumpRuntimeInfo() {
                unsigned int  qps  = meisterwerk::core::message::que.peak();
                unsigned int  qln  = meisterwerk::core::message::que.length();
                unsigned long smdp = meisterwerk::core::baseapp::_app->sched.msgTime.getcount();
                unsigned long smqt = meisterwerk::core::baseapp::_app->sched.msgTime.getms();
                unsigned long stkc = meisterwerk::core::baseapp::_app->sched.tskTime.getcount();
                unsigned long stkt = meisterwerk::core::baseapp::_app->sched.tskTime.getms();
                unsigned long slit = meisterwerk::core::baseapp::_app->sched.allTime.getms();
                String        pre  = "dumper(" + entName + ") Runtime Information - ";

                DBG( pre + "Memory(Free Heap=" + ESP.getFreeHeap() + " bytes), Queue(cur=" + qln + " / max=" + qps +
                     "), Scheduler(msg=" + smdp + " / tasks=" + stkc + " / msg_time=" + smqt +
                     " ms / task_time=" + stkt + " ms / life_time=" + slit + " ms)" );
            }

            void dumpTaskInfo() {
                if ( meisterwerk::core::baseapp::_app ) {
                    meisterwerk::core::baseapp::_app->sched.dumpInfo( "dumper(" + entName + ") " );
                }
            }
#else
            // fake entity. Will neither register nor do anything
            dumper( String name = "", int _iMinDumpSecs = 0, String _debugButton = "" )
                : meisterwerk::core::entity( "" ) {
            }

            bool registerEntity( unsigned long minMicroSecs = 0, unsigned int priority = 3 ) {
                return true;
            }
#endif
        };
    } // namespace util
} // namespace meisterwerk
