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
                        meisterwerk::core::T_PRIO priority     = meisterwerk::core::PRIORITY_NORMAL )
                : button( name, minMicroSecs, priority ), minLongMs{minLongMs}, minExtraLongMs{minExtraLongMs} {
                lastState    = NONE;
                lastDuration = 0;
            }

            /*
                        virtual void onGetState( JsonObject &request, JsonObject &response ) override {
                            response["type"]      = "button/pushbutton";
                            response["state"]     = getStateString( lastState );
                            response["duration"]  = lastDuration;
                            response["long"]      = minLongMs;
                            response["extraLong"] = minExtraLongMs;
                        }

                        virtual bool onSetState( JsonObject &request, JsonObject &response ) override {
                            bool        bChanged    = false;
                            JsonVariant toLong      = request["long"];
                            JsonVariant toExtraLong = request["extraLong"];
                            if ( willSetStateU( toLong, minLongMs ) ) {
                                bChanged         = true;
                                minLongMs        = toLong.as<unsigned long>();
                                response["long"] = minLongMs;
                            }
                            if ( willSetStateU( toExtraLong, minExtraLongMs ) ) {
                                bChanged              = true;
                                minExtraLongMs        = toExtraLong.as<unsigned long>();
                                response["extraLong"] = minExtraLongMs;
                            }
                            return bChanged;
                        }
            */
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
                String x     = getStateString( state );
                String s     = "\"state\":\"" + x + "\"";
                String d     = "\"duration\":" + String( duration );
                // notify status change - generic
                // publish( ownTopic( "state" ), "{" + s + "," + d + "}" );
                // notify status change - FHEM style
                publish( entName + "/" + x, "{" + d + "}" );
            }
        };
    } // namespace base
} // namespace meisterwerk
