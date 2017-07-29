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
#include "../core/topic.h"
#include "../util/eggtimer.h"

namespace meisterwerk {
    namespace base {

        class onoff : public meisterwerk::core::entity {
            public:
            bool                        state      = false;
            bool                        stateNext  = false;
            meisterwerk::util::eggtimer stateTimer = 0;

            onoff( String name, unsigned long minMicroSecs = 250000,
                   meisterwerk::core::T_PRIO priority = meisterwerk::core::PRIORITY_NORMAL )
                : meisterwerk::core::entity( name, minMicroSecs, priority ) {
                // The state timer had a millisecond resolution. For most
                // purposes a 250 ms resolution sbhould be enought, so take
                // this if a default. If needed an entity can be initialized
                // to a higher resolution
            }

            // ABSTRACT METHOD: This override must be implemented in derived classes
            virtual bool onSwitch( bool newstate ) = 0;

            virtual void setup() override {
                meisterwerk::core::entity::setup();
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

            virtual void receive( const char *origin, const char *topic, const char *msg ) override {
                // String t( topic );
                meisterwerk::core::Topic t( topic );
                DynamicJsonBuffer        reqBuffer( 300 );
                DynamicJsonBuffer        resBuffer( 300 );
                JsonObject &             req        = reqBuffer.parseObject( msg );
                JsonObject &             res        = resBuffer.createObject();
                JsonVariant              toDuration = req["duration"];

                if ( t == ownTopic( "on" ) ) {
                    setState( true, toDuration.as<unsigned long>(), res, true );
                } else if ( t == ownTopic( "off" ) ) {
                    setState( true, toDuration.as<unsigned long>(), res, true );
                } else if ( t == ownTopic( "toggle" ) ) {
                    setState( !state, toDuration.as<unsigned long>(), res, true );
                }
            }

            /*
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
            */

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
                    String buffer;
                    response.printTo( buffer );
                    publish( ownTopic( "state" ), buffer );
                }
                return bChanged;
            }

            String ownTopic( String subtopic ) const {
                return entName + "/" + subtopic;
            }

            bool isOwnTopic( String topic, String subtopic ) const {
                return topic == entName + "/" + subtopic;
            }
        };
    } // namespace base
} // namespace meisterwerk