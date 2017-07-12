// pushbutton.h - Base class for a push button
//
// This is the declaration of the base class for a
// push button. A pushbutton publishes an event when
// when its push state changes:
//
// Messages are published when the button is pushed
// and when the button is released. The message content
// is the duration of the previous state
//
// Publish:
// - "NAME/press": when the button is pressed
// - "NAME/short": when the button is released before the
//                 minimum time for long presses
// - "NAME/long":  when the button is released after the
//                 minimum time for long presses
// - "NAME/extralong":  when the button is released after the
//                      minimum time for extra long presses

#pragma once

// dependencies
#include "button.h"

namespace meisterwerk {
    namespace base {
        class pushbutton : public button {
            public:
            unsigned int minLongMs;
            unsigned int minExtraLongMs;

            pushbutton( String name, unsigned int _minLongMs = 0, unsigned int _minExtraLongMs = 0 )
                : button( name ) {
                minLongMs      = _minLongMs;
                minExtraLongMs = _minExtraLongMs;
            }

            virtual void onSetup() override {
                button::onSetup();
                subscribe( entName + "/config" );
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                if ( topic == entName + "/config" ) {
                    // XXX: Parse JSON data and set minLongMs,minExtraLongMs
                }
            }

            virtual void onChange( bool toState, unsigned long duration ) override {
                if ( toState ) {
                    publish( entName + "/press", "{d:" + String( duration / 1000 ) + "}" );
                } else {
                    if ( minExtraLongMs && duration / 1000 > minExtraLongMs ) {
                        publish( entName + "/extralong", "{d:" + String( duration / 1000 ) + "}" );
                    } else if ( minLongMs && duration / 1000 > minLongMs ) {
                        publish( entName + "/long", "{d:" + String( duration / 1000 ) + "}" );
                    } else {
                        publish( entName + "/short", "{d:" + String( duration / 1000 ) + "}" );
                    }
                }
            }
        };
    } // namespace base
} // namespace meisterwerk
