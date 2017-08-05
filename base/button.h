// button.h - Base class for a button
//
// This is the declaration of the base class for a
// button. A button publishes an event when its
// push state changes:
//
// Messages are published when the button is pushed
// and when the button is released. The message content
// is the duration of the previous state.

#pragma once

// dependencies
#include "../core/entity.h"
#include "../util/stopwatch.h"

namespace meisterwerk {
    namespace base {
        class button : public meisterwerk::core::jentity {
            protected:
            bool                         fromState;
            meisterwerk::util::stopwatch lastChange;

            public:
            button( String name, unsigned long minMicroSecs, core::T_PRIO priority = core::PRIORITY_NORMAL,
                    unsigned int wordListSize = 6 )
                : meisterwerk::core::jentity( name, minMicroSecs, priority, wordListSize ) {
                fromState = false;
            }

            virtual void setup() override {
                Notifies( "press" );
                Notifies( "release" );

                Reading( "state" );
            }

            virtual void onGetValue( String value, JsonObject &params, JsonObject &data ) override {
                if ( value == "info" ) {
                    data["type"]     = "button";
                    data["state"]    = fromState ? "press" : "release";
                    data["duration"] = lastChange.getduration();
                    notify( value, data );
                } else if ( value == "state" ) {
                    data["state"]    = fromState ? "press" : "release";
                    data["duration"] = lastChange.getduration();
                    notify( value, data );
                }
            }

            virtual void onChange( bool toState, unsigned long duration ) {
                DynamicJsonBuffer resBuffer( 256 );
                JsonObject &      data = resBuffer.createObject();
                prepareData( data );
                data["duration"] = duration;
                notify( toState ? "press" : "release", data );
            }

            // internal
            protected:
            void change( bool toState ) {
                if ( fromState != toState ) {
                    // handle state change
                    onChange( toState, lastChange.getleap() );
                    fromState = toState;
                }
            }
        };
    } // namespace base
} // namespace meisterwerk
