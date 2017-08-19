// dumper.h - A helper entity for dumping system info
//
// This is the declaration of a class performing
// information dumps emitted through the serial
// interface.
// The class can be configured to dump information
// at regular intervals or only on request.
// The class works only if #define DEBUG

#pragma once

// dependencies
#include "../core/entity.h"
#include "../core/topic.h"
#include "../util/metronome.h"

namespace meisterwerk {
    namespace util {

#ifdef _MW_DEBUG
        class dumper : public core::entity {
            public:
            util::metronome autodump;
            String          debugButton;

            dumper( String name = "dmp", unsigned long autodump = 0, String debugButton = "dbg",
                    unsigned long minMicroSecs = 250000, core::T_PRIO priority = core::PRIORITY_NORMAL )
                : core::entity( name, minMicroSecs, priority ), autodump{autodump}, debugButton{debugButton} {
            }

            void setup() override {
                // explicit commands
                subscribe( "+/dump" );
                subscribe( "+/sysinfo" );
                subscribe( "+/taskinfo" );
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

            virtual void receive( const char *origin, const char *tpc, const char *msg ) override {
                core::Topic topic( tpc );
                // process my own subscriptions
                if ( topic.match( "+/dump" ) || topic == debugButton + "/short" ) {
                    dumpRuntimeInfo();
                } else if ( topic.match( "+/sysinfo" ) || topic == debugButton + "/long" ) {
                    dumpSystemInfo();
                } else if ( topic.match( "+/taskinfo" ) || topic == debugButton + "/extralong" ) {
                    dumpTaskInfo();
                }
            }

            void dumpSystemInfo() {
                String                     pre   = "dumper(" + entName + ") ";
                const __FlashStringHelper *bytes = F( " bytes" );
                DBG( "" );
                DBG( pre + F( "System Information:" ) );
                DBG( pre + F( "-------------------" ) );
#ifdef ESP8266
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
#else
#endif
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
#ifdef ESP8266
                unsigned long fmem = ESP.getFreeHeap();
#else
                unsigned long fmem = 0;
#endif

                DBG( pre + "Memory(Free Heap=" + fmem + " bytes), Queue(cur=" + qln + " / max=" + qps +
                     "), Scheduler(msg=" + smdp + " / tasks=" + stkc + " / msg_time=" + smqt +
                     " ms / task_time=" + stkt + " ms / life_time=" + slit + " ms)" );
            }

            void dumpTaskInfo() {
                if ( meisterwerk::core::baseapp::_app ) {
                    meisterwerk::core::baseapp::_app->sched.dumpInfo( "dumper(" + entName + ") " );
                }
            }
        };
#else
        class dumper {
            public:
            // fake entity. Will neither register nor do anything
            dumper( String name = "dmp", unsigned long autodump = 0, String debugButton = "dbg",
                    unsigned long             minMicroSecs = 250000,
                    meisterwerk::core::T_PRIO priority     = meisterwerk::core::PRIORITY_NORMAL ) {
            }
        };
#endif
    } // namespace util
} // namespace meisterwerk
