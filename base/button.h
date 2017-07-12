// button.h - Base class for a button
//
// This is the declaration of the base class for a
// button. A button publishes an event when its
// push state changes:
//
// Messages are published when the button is pushed
// and when the button is released. The message content
// is the duration of the previous state.
//
// Publish:
// - "NAME/push"
// - "NAME/release"
//
// Subscribe:
// - "NAME/getstate"

#pragma once

// dependencies
#include "../core/entity.h"
#include "../util/timebudget.h"

namespace meisterwerk {
    namespace base {
        class button : public meisterwerk::core::entity {
            public:
            bool          fromState;
            unsigned long lastChange;

            button( String name ) : meisterwerk::core::entity( name ) {
                fromState  = false;
                lastChange = 0;
            }

            virtual void onRegister() override {
                subscribe( entName + "/getstate" );
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                // XXX: implement getstate.
            }

            virtual void onChange( bool toState, unsigned long duration ) {
                // fire a message
                if ( toState ) {
                    // press
                    publish( entName + "/press", "{d:" + String( duration / 1000 ) + "}" );
                } else {
                    // release
                    publish( entName + "/release", "{d:" + String( duration / 1000 ) + "}" );
                }
            }

            virtual void press() {
                change( true );
            }

            virtual void release() {
                change( false );
            }

            virtual void change( bool toState ) {
                if ( fromState != toState ) {
                    unsigned long last = micros();
                    unsigned long duration =
                        meisterwerk::util::timebudget::delta( lastChange, last );
                    lastChange = last;
                    fromState  = toState;
                    // handle state change
                    onChange( toState, duration );
                }
            }
        };
    } // namespace base
} // namespace meisterwerk
