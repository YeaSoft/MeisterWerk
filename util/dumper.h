// dumper.h - A helper entity for dumping system info
//
// This is the declaration of a class performing
// information dumps emitted through the serial
// interface.
// The class can be configured to dump information
// at regular intervals or only on request.
// The class works only if #define DEBUG

// dependencies
#include "../core/entity.h"

// dependencies
#include "../core/entity.h"

namespace meisterwerk {
    namespace util {

        class dumper : public meisterwerk::core::entity {
            public:
#ifdef DEBUG
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

            bool registerEntity() {
                return true;
            }
#endif

#ifdef DEBUG
            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 1000000 );
            }

            void onSetup() override {
                dumpSystemInfo();
                // explicit dump messages
                subscribe( "*/dump" );
                subscribe( "*/sysinfo" );
                subscribe( "*/taskinfo" );
                // Debug Button
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

            virtual void onReceiveMessage( String topic, const char *pBuf, unsigned int len ) {
                if ( topic == debugButton + "/short" ) {
                    dumpRuntimeInfo();
                } else if ( topic == debugButton + "/long" ) {
                    dumpSystemInfo();
                } else if ( topic == debugButton + "/extralong" ) {
                    dumpTaskInfo();
                }
            }

            void dumpSystemInfo() {
                String pre = "dumper(" + entName + ") ";
                DBG( "System Information:" );
                DBG( "-------------------" );
                DBG( pre + "Chip ID: " + ESP.getChipId() );
                DBG( pre + "Core Verion: " + ESP.getCoreVersion() );
                DBG( pre + "SDK Verion: " + ESP.getSdkVersion() );
                DBG( pre + "CPU Frequency: " + ESP.getCpuFreqMHz() + " MHz" );
                DBG( pre + "Program Size: " + ESP.getSketchSize() + " bytes" );
                DBG( pre + "Program Free: " + ESP.getFreeSketchSpace() + " bytes" );
                DBG( pre + "Flash Chip ID: " + ESP.getFlashChipId() );
                DBG( pre + "Flash Chip Size: " + ESP.getFlashChipSize() + " bytes" );
                DBG( pre + "Flash Chip Real Size: " + ESP.getFlashChipRealSize() + " bytes" );
                DBG( pre + "Flash Chip Speed: " + ESP.getFlashChipSpeed() + " hz" );
                DBG( pre + "Last Reset Reason: " + ESP.getResetReason() );
            }

            void dumpRuntimeInfo() {
                unsigned int qps = meisterwerk::core::message::que.peak();
                unsigned int qln = meisterwerk::core::message::que.length();
                String       pre = "dumper(" + entName + ") ";
                DBG( pre + "Free Heap: " + ESP.getFreeHeap() + " bytes, Queue(cur/max): " + qln +
                     "/" + qps );
            }

            void dumpTaskInfo() {
                String pre = "dumper(" + entName + ") ";
                DBG( "Task Information" );
                DBG( "----------------" );
                // XXX dump task statistics
            }
#endif
        };
    }
}