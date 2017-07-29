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
#include "../core/entity.h"
#include "../util/eggtimer.h"

namespace meisterwerk {
    namespace base {

        class onoff : public meisterwerk::core::entity {
            public:
            bool                        state      = false;
            bool                        stateNext  = false;
            meisterwerk::util::eggtimer stateTimer = 0;

            onoff( String name ) : meisterwerk::core::entity( name ) {
            }

            bool registerEntity( unsigned long minMicroSecs = 250000, unsigned int priority = 3 ) {
                // The state timer had a millisecond resolution. For most
                // purposes a 250 ms resolution sbhould be enough, so take
                // this if a default. Iff needed an entity can be initialized
                // to a higher resolution
                return meisterwerk::core::entity::registerEntity( minMicroSecs );
            }

            // ABSTRACT METHOD: This override must be implemented in derived classes
            virtual bool onSwitch( bool newstate ) = 0;

            virtual void onRegister() override {
                meisterwerk::core::entity::onRegister();
                // FHEM style commands
                subscribe( ownTopic( "on" ) );
                subscribe( ownTopic( "off" ) );
                subscribe( ownTopic( "toggle" ) );
            }

            virtual void loop() override {
                if ( stateTimer > 0 ) {
                    // timed state change requested
                    if ( stateTimer.isexpired() ) {
                        // perform action
                        if ( state != stateNext ) {
                            DynamicJsonBuffer resBuffer( 300 );
                            JsonObject &      res = resBuffer.createObject();
                            setState( stateNext, 0, res, true );
                        }
                    }
                }
            }

            virtual bool onReceive( String origin, String topic, JsonObject &request, JsonObject &response ) override {
                if ( meisterwerk::core::entity::onReceive( origin, topic, request, response ) ) {
                    return true;
                }
                // process my own subscriptions
                JsonVariant toDuration = request["duration"];
                if ( isOwnTopic( topic, "on" ) ) {
                    setState( true, toDuration.as<unsigned long>(), response, true );
                    return true;
                } else if ( isOwnTopic( topic, "/off" ) ) {
                    setState( false, toDuration.as<unsigned long>(), response, true );
                    return true;
                } else if ( isOwnTopic( topic, "toggle" ) ) {
                    setState( !state, toDuration.as<unsigned long>(), response, true );
                    return true;
                }
                return false;
            }

            virtual void onGetState( JsonObject &request, JsonObject &response ) override {
                response["type"]     = "onoff";
                response["state"]    = state;
                response["duration"] = stateTimer.getduration();
            }

            virtual bool onSetState( JsonObject &request, JsonObject &response ) override {
                JsonVariant toState    = request["state"];
                JsonVariant toDuration = request["duration"];
                if ( willSetStateB( toState, state ) ) {
                    return setState( toState.as<bool>(), toDuration.as<unsigned long>(), response );
                } else if ( willSetStateU( toDuration, stateTimer.getduration() ) ) {
                    return setState( state, toDuration.as<unsigned long>(), response );
                }
                return false;
            }

            bool setState( bool newstate, unsigned long duration, JsonObject &response, bool publishState = false ) {
                bool bChanged = false;
                if ( newstate != state ) {
                    if ( onSwitch( newstate ) ) {
                        stateTimer           = duration;
                        stateNext            = duration ? !newstate : newstate;
                        state                = newstate;
                        response["state"]    = newstate;
                        response["duration"] = duration;
                        bChanged             = true;
                    } else {
                        DBG( entName + ": Hardware failure while switching state" );
                    }
                } else if ( duration != stateTimer ) {
                    // do not change the state, but the duration for this state
                    stateTimer           = duration;
                    stateNext            = !state;
                    response["duration"] = duration;
                    bChanged             = true;
                }
                if ( bChanged ) {
                    publish( ownTopic( "state" ), response );
                }
                return bChanged;
            }
        };
    } // namespace base
} // namespace meisterwerk