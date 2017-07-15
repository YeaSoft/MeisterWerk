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

            // ABSTRACT CLASS: This override must be implemented in derived classes
            virtual bool onSwitch( bool newstate ) = 0;

            virtual void onRegister() override {
                // standard commands
                subscribe( entName + "/getstate" );
                subscribe( entName + "/setstate" );
                // convenience shortcuts
                subscribe( entName + "/on" );
                subscribe( entName + "/off" );
                subscribe( entName + "/toggle" );
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                DynamicJsonBuffer jsonBuffer( 300 );
                JsonObject &      root = jsonBuffer.parseObject( msg );
                if ( !root.success() ) {
                    DBG( "Invalid JSON received!" );
                    return;
                }
                if ( topic == entName + "/on" ) {
                    setState( true, root["duration"].as<unsigned long>() );
                } else if ( topic == entName + "/off" ) {
                    setState( false, root["duration"].as<unsigned long>() );
                } else if ( topic == entName + "/toggle" ) {
                    setState( !state, root["duration"].as<unsigned long>() );
                } else if ( topic == entName + "/getstate" ) {
                    getState();
                } else if ( topic == entName + "/setstate" ) {
                    setState( root["state"].as<bool>(), root["duration"].as<unsigned long>() );
                }
            }

            virtual void onLoop( unsigned long timer ) override {
                if ( stateTimer > 0 ) {
                    if ( stateTimer.isexpired() ) {
                        // perform action
                        if ( state != stateNext ) {
                            setState( stateNext, 0 );
                        }
                    }
                }
            }

            void setState( bool newstate, unsigned long duration ) {
                if ( newstate != state ) {
                    if ( onSwitch( newstate ) ) {
                        stateTimer = duration;
                        stateNext  = duration ? !newstate : newstate;
                        state      = newstate;
                        publish( entName + "/state", getStateJSON() );
                    } else {
                        DBG( entName + ": Hardware failure while switching state" );
                    }
                } else if ( duration != stateTimer ) {
                    // do not change the state, but the duration for this state
                    stateTimer = duration;
                    stateNext  = !state;
                    publish( entName + "/state", getStateJSON() );
                }
            }

            void getState() {
                publish( entName + "/state", getStateJSON() );
            }

            // internal
            private:
            String getStateJSON() {
                char              szBuffer[256];
                DynamicJsonBuffer jsonBuffer( 200 );
                JsonObject &      root = jsonBuffer.createObject();
                root["state"]          = state;
                root["duration"]       = stateTimer.getduration();
                root.printTo( szBuffer );
                return szBuffer;
            }
        };
    } // namespace base
} // namespace meisterwek