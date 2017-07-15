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
            protected:
            // configuration
            unsigned long minLongMs;
            unsigned long minExtraLongMs;
            // state
            typedef enum { NONE, SHORT, LONG, EXTRALONG } STATE;
            STATE         lastState;
            unsigned long lastDuration;

            pushbutton( String name, unsigned int _minLongMs = 0, unsigned int _minExtraLongMs = 0 )
                : button( name ) {
                lastState      = NONE;
                lastDuration   = 0;
                minLongMs      = _minLongMs;
                minExtraLongMs = _minExtraLongMs;
            }

            virtual void onRegister() override {
                button::onRegister();
                // standard conditionally mandatory commands
                subscribeme( "getconfig" );
                subscribeme( "setconfig" );
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                if ( topic == entName + "/getstate" ) {
                    publish( entName + "/state", getStateJSON() );
                } else if ( topic == entName + "/getconfig" ) {
                    publish( entName + "/config", getConfigJSON() );
                } else if ( topic == entName + "/setconfig" ) {
                    DynamicJsonBuffer jsonBuffer( 300 );
                    JsonObject &      root = jsonBuffer.parseObject( msg );
                    if ( !root.success() ) {
                        DBG( "Invalid JSON received!" );
                        return;
                    }
                    minLongMs      = root["long"].as<unsigned long>();
                    minExtraLongMs = root["extraLong"].as<unsigned long>();
                }
            }

            virtual void onChange( bool toState, unsigned long duration ) override {
                if ( toState ) {
                    button::onChange( toState, duration );
                } else {
                    if ( minExtraLongMs && duration > minExtraLongMs ) {
                        setState( EXTRALONG, duration );
                    } else if ( minLongMs && duration > minLongMs ) {
                        setState( LONG, duration );
                    } else {
                        setState( SHORT, duration );
                    }
                }
            }

            // internal
            protected:
            String getStateString( STATE state ) const {
                switch ( state ) {
                default:
                    return "none";
                case SHORT:
                    return "short";
                case LONG:
                    return "long";
                case EXTRALONG:
                    return "extralong";
                }
            }

            void setState( STATE state, unsigned int duration ) {
                lastState    = state;
                lastDuration = duration;
                publish( entName + "/" + getStateString( state ),
                         "{\"duration\":" + String( duration ) + "}" );
            }

            String getStateJSON() const {
                char              szBuffer[256];
                DynamicJsonBuffer jsonBuffer( 200 );
                JsonObject &      root = jsonBuffer.createObject();
                root["state"]          = getStateString( lastState );
                root["duration"]       = lastDuration;
                root.printTo( szBuffer );
                return szBuffer;
            }

            String getConfigJSON() const {
                char              szBuffer[256];
                DynamicJsonBuffer jsonBuffer( 200 );
                JsonObject &      root = jsonBuffer.createObject();
                root["long"]           = minLongMs;
                root["extraLong"]      = minExtraLongMs;
                root.printTo( szBuffer );
                return szBuffer;
            }
        };
    } // namespace base
} // namespace meisterwerk
