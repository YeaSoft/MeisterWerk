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
            button( String name, unsigned long minMicroSecs,
                    meisterwerk::core::T_PRIO priority = meisterwerk::core::PRIORITY_NORMAL )
                : meisterwerk::core::entity( name, minMicroSecs, priority ) {
                fromState = false;
            }

            virtual void onChange( bool toState, unsigned long duration ) {
                String x = toState ? "press" : "release";
                String s = "\"state\":\"" + x + "\"";
                String d = "\"duration\":" + String( duration );
                // notify status change - generic
                // publish( ownTopic( "state" ), "{" + s + "," + d + "}" );
                // notify status change - FHEM style
                publish( entName + "/" + x, "{" + d + "}" );
            }

            // virtual void onGetState( JsonObject &request, JsonObject &response ) override {
            //    response["type"]     = "button";
            //    response["state"]    = fromState ? "press" : "release";
            //    response["duration"] = lastChange.getduration();
            //}

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
