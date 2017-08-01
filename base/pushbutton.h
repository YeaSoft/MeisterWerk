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
            enum STATE { NONE, SHORT, LONG, EXTRALONG };
            STATE         lastState;
            unsigned long lastDuration;

            pushbutton( String name, unsigned int minLongMs = 0, unsigned int minExtraLongMs = 0,
                        unsigned long             minMicroSecs = 0,
                        meisterwerk::core::T_PRIO priority     = meisterwerk::core::PRIORITY_NORMAL,
                        unsigned int              wordListSize = 8 )
                : button( name, minMicroSecs, priority, wordListSize ), minLongMs{minLongMs}, minExtraLongMs{
                                                                                                  minExtraLongMs} {
                lastState    = NONE;
                lastDuration = 0;
            }

            virtual void setup() override {
                Notifies( "press" );
                Notifies( "short" );
                Notifies( "long" );
                Notifies( "extralong" );

                Setting( "longduration" );
                Setting( "extralongduration" );
            }

            virtual void onGetValue( String value, JsonObject &params, JsonObject &data ) override {
                if ( value == "info" ) {
                    data["type"]              = "pushbutton";
                    data["longduration"]      = minLongMs;
                    data["extralongduration"] = minExtraLongMs;
                    notify( value, data );
                } else if ( value == "longduration" ) {
                    data["longduration"] = minLongMs;
                    notify( value, data );
                } else if ( value == "extralongduration" ) {
                    data["extralongduration"] = minExtraLongMs;
                    notify( value, data );
                }
            }

            virtual void onSetValue( String value, JsonObject &params, JsonObject &data ) override {
                if ( value == "longduration" ) {
                    minLongMs = params["longduration"].as<unsigned long>();
                } else if ( value == "extralongduration" ) {
                    minExtraLongMs = params["extralongduration"].as<unsigned long>();
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

                DynamicJsonBuffer resBuffer( 256 );
                JsonObject &      data = resBuffer.createObject();
                prepareData( data );
                data["duration"] = duration;
                notify( getStateString( state ), data );
            }
        };
    } // namespace base
} // namespace meisterwerk
