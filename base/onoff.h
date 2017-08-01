// onoff.h - Base class for a on and off switch
//
// This is the declaration of the base class for a
// switch. A switch ...

// Standard subscriptions:
// - getstate
// - setstate

// Emits:
// - state

#pragma once

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../core/jentity.h"
#include "../core/topic.h"
#include "../util/eggtimer.h"

namespace meisterwerk {
    namespace base {

        class onoff : public meisterwerk::core::jentity {
            public:
            bool                        state      = false;
            bool                        stateNext  = false;
            meisterwerk::util::eggtimer stateTimer = 0;

            onoff( String name, unsigned long minMicroSecs = 250000,
                   meisterwerk::core::T_PRIO priority = meisterwerk::core::PRIORITY_NORMAL )
                : meisterwerk::core::jentity( name, minMicroSecs, priority ) {
                // The state timer had a millisecond resolution. For most
                // purposes a 250 ms resolution sbhould be enought, so take
                // this if a default. If needed an entity can be initialized
                // to a higher resolution
            }

            // ABSTRACT METHOD: This override must be implemented in derived classes
            virtual bool onSwitch( bool newstate ) = 0;

            virtual void setup() override {
                Reaction( "on" );
                Reaction( "off" );
                Reaction( "toggle" );

                SettableState( "state" );
            }

            virtual void loop() override {
                if ( stateTimer > 0 ) {
                    // timed state change requested
                    if ( stateTimer.isexpired() ) {
                        // perform scheduled action
                        if ( state != stateNext ) {
                            DynamicJsonBuffer resBuffer( 256 );
                            JsonObject &      data = resBuffer.createObject();
                            prepareData( data );
                            setState( stateNext, 0, data, true );
                        }
                    }
                }
            }

            virtual void onReaction( String cmd, JsonObject &params, JsonObject &data ) override {
                if ( cmd == "on" ) {
                    setState( true, params["duration"].as<unsigned long>(), data, true );
                } else if ( cmd == "off" ) {
                    setState( false, params["duration"].as<unsigned long>(), data, true );
                } else if ( cmd == "toggle" ) {
                    setState( !state, params["duration"].as<unsigned long>(), data, true );
                }
            }

            virtual void onGetValue( String value, JsonObject &params, JsonObject &data ) override {
                if ( value == "info" ) {
                    data["type"]     = "onoff";
                    data["state"]    = state;
                    data["duration"] = stateTimer.getduration();
                    notify( "info", data );
                } else if ( value == "state" ) {
                    data["state"]    = state;
                    data["duration"] = stateTimer.getduration();
                    notify( "state", data );
                }
            }

            virtual void onSetValue( String value, JsonObject &params, JsonObject &data ) override {
                if ( value == "state" ) {
                    setState( params["state"].as<bool>(), params["duration"].as<unsigned long>(), data );
                }
            }

            bool setState( bool newstate, unsigned long duration, JsonObject &data, bool publishState = false ) {
                bool bChanged = false;
                if ( newstate != state ) {
                    if ( onSwitch( newstate ) ) {
                        stateTimer       = duration;
                        stateNext        = duration ? !newstate : newstate;
                        state            = newstate;
                        data["state"]    = newstate;
                        data["duration"] = duration;
                        bChanged         = true;
                    } else {
                        DBG( entName + ": Hardware failure while switching state" );
                    }
                } else if ( duration != stateTimer ) {
                    // do not change the state, but the duration for this state
                    stateTimer       = duration;
                    stateNext        = !state;
                    data["duration"] = duration;
                    bChanged         = true;
                }
                if ( bChanged && publishState ) {
                    notify( "state", data );
                }
                return bChanged;
            }
        };
    } // namespace base
} // namespace meisterwerk