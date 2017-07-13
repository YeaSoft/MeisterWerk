// onoff.h - Base class for a on and off switch
//
// This is the declaration of the base class for a
// switch. A switch ...

// Standard subscriptions:
// - getstate
// - setstate

// Emits:
// - statechange

#pragma once

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../core/entity.h"

namespace meisterwerk {
    namespace base {

        class onoff : public meisterwerk::core::entity {
            public:
            bool         state      = false;
            bool         stateNext  = false;
            unsigned int stateTimer = 0;

            onoff( String name ) : meisterwerk::core::entity( name ) {
            }

            bool registerEntity() {
                // the state timer has a second resolution. It is enough
                // to check it each 250 ms
                return meisterwerk::core::entity::registerEntity( 1000000 );
            }

            // ABSTRACT CLASS: This override must be implemented in derived classes
            virtual bool onSwitch( bool newstate ) = 0;

            virtual void onRegister() override {
                subscribe( entName + "/on" );
                subscribe( entName + "/off" );
                subscribe( entName + "/toggle" );
                subscribe( entName + "/getstate" );
                subscribe( entName + "/setstate" );
                subscribe( entName + "/getconfig" );
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                DynamicJsonBuffer jsonBuffer( 200 );
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
                if ( stateTimer ) {
                    --stateTimer;
                    if ( stateTimer == 0 ) {
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
                        publish( entName + "/state", makeState( state, stateTimer ) );
                    } else {
                        DBG( entName + ": Hardware failure while switching state" );
                    }
                } else if ( duration != stateTimer ) {
                    // do not change the state, but the duration for this state
                    stateTimer = duration;
                    stateNext  = !state;
                    publish( entName + "/state", makeState( state, stateTimer ) );
                }
            }

            void getState() {
                publish( entName + "/state", makeState( state, stateTimer ) );
            }

            // internal
            private:
            String makeState( bool newstate, unsigned long duration ) {
                char              szBuffer[256];
                DynamicJsonBuffer jsonBuffer( 200 );
                JsonObject &      root = jsonBuffer.createObject();
                root["state"]          = newstate;
                root["duration"]       = duration;
                root.printTo( szBuffer );
                return szBuffer;
            }
        };
    } // namespace base
} // namespace meisterwek