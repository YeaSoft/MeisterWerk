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
        class button : public meisterwerk::core::entity {
            protected:
            bool                         fromState;
            meisterwerk::util::stopwatch lastChange;

            public:
            button( String name ) : meisterwerk::core::entity( name ) {
                fromState = false;
            }

            virtual void onChange( bool toState, unsigned long duration ) {
                String x = toState ? "press" : "release";
                String s = "\"state\":\"" + x + "\"";
                String d = "\"duration\":" + String( duration );
                // notify status change - generic
                publish( ownTopic( "state" ), "{" + s + "," + d + "}" );
                // notify status change - FHEM style
                publish( ownTopic( x ), "{" + d + "}" );
            }

            virtual void onGetState( JsonObject &request, JsonObject &response ) override {
                response["type"]     = "button";
                response["state"]    = fromState ? "press" : "release";
                response["duration"] = lastChange.getduration();
            }

            virtual bool onSetState( JsonObject &request, JsonObject &response ) override {
                // button has no settable state
                return false;
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
