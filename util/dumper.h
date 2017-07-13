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

namespace meisterwerk {
    namespace util {

        class dumper : public meisterwerk::core::entity {
            public:
#ifdef _DEBUG
            String debugButton;
            int    iMinDumpSecs;
            int    iCount = 0;

            dumper( String name = "dmp", int _iMinDumpSecs = 0, String _debugButton = "dbg" )
                : meisterwerk::core::entity( name ) {
                debugButton  = _debugButton;
                iMinDumpSecs = _iMinDumpSecs;
            }
#else
            dumper( String name = "", int _iMinDumpSecs = 0, String _debugButton = "" )
                : meisterwerk::core::entity( "" ) {
            }

            bool registerEntity( unsigned long minMicroSecs = 0, unsigned int priority = 3 ) {
                // never register
                return true;
            }
#endif

#ifdef _DEBUG
            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 1000000 );
            }

            void onRegister() override {
                dumpSystemInfo();
                // explicit commands
                subscribe( "*/dump" );
                subscribe( "*/sysinfo" );
                subscribe( "*/taskinfo" );
                // events from debug Button
                subscribe( debugButton + "/short" );
                subscribe( debugButton + "/long" );
                subscribe( debugButton + "/extralong" );
            }

            virtual void onLoop( unsigned long ticker ) override {
                if ( iMinDumpSecs ) {
                    if ( iCount < 1 ) {
                        dumpRuntimeInfo();
                        iCount = iMinDumpSecs;
                    } else {
                        --iCount;
                    }
                }
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                if ( topic == debugButton + "/short" ) {
                    dumpRuntimeInfo();
                } else if ( topic == debugButton + "/long" ) {
                    dumpSystemInfo();
                } else if ( topic == debugButton + "/extralong" ) {
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

                DBG( pre + "Memory(Free Heap=" + ESP.getFreeHeap() + " bytes), Queue(cur=" + qln +
                     " / max=" + qps + "), Scheduler(msg=" + smdp + " / tasks=" + stkc +
                     " / msg_time=" + smqt + " ms / task_time=" + stkt + " ms / life_time=" + slit +
                     " ms)" );
            }

            void dumpTaskInfo() {
                if ( meisterwerk::core::baseapp::_app ) {
                    meisterwerk::core::baseapp::_app->sched.dumpInfo( "dumper(" + entName + ") " );
                }
            }
#endif
        };
    } // namespace util
} // namespace meisterwerk
