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

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../core/entity.h"
#include "../util/stopwatch.h"

namespace meisterwerk {
    namespace base {
        class button : public meisterwerk::core::entity {
            protected:
            // state
            bool                         fromState;
            meisterwerk::util::stopwatch lastChange;

            button( String name ) : meisterwerk::core::entity( name ) {
                fromState = false;
            }

            virtual void onRegister() override {
                subscribe( entName + "/getstate" );
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                if ( topic == entName + "/getstate" ) {
                    publish( entName + "/state", getStateJSON() );
                }
            }

            virtual void onChange( bool toState, unsigned long duration ) {
                // fire a message
                publish( entName + ( toState ? "/press" : "/release" ),
                         "{\"duration\":" + String( duration ) + "}" );
            }

            // internal
            protected:
            void change( bool toState ) {
                if ( fromState != toState ) {
                    // handle state change
                    onChange( toState, lastChange.getloop() );
                    fromState = toState;
                }
            }

            String getStateJSON() const {
                char              szBuffer[256];
                DynamicJsonBuffer jsonBuffer( 200 );
                JsonObject &      root = jsonBuffer.createObject();
                root["state"]          = fromState ? "press" : "release";
                root["duration"]       = lastChange.getduration();
                root.printTo( szBuffer );
                return szBuffer;
            }
        };
    } // namespace base
} // namespace meisterwerk
