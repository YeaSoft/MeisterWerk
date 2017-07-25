// messagespy.h - A helper entity for message monitoring
//
// This is the declaration of a monitoring class for
// internal message monitoring. The messages are
// emitted through the serial interface
// The class works only if #define DEBUG

#pragma once

// dependencies
#include "../core/entity.h"

namespace meisterwerk {
    namespace util {

        class messagespy : public meisterwerk::core::entity {
            private:
#ifdef _DEBUG
            String tmpSubscribedTopic;
#endif

            public:
#ifdef _DEBUG
            messagespy( String name = "spy", String subscription = "*" )
                : meisterwerk::core::entity( name ) {
                tmpSubscribedTopic = subscription;
            }

            void onRegister() override {
                subscribe( tmpSubscribedTopic );
                tmpSubscribedTopic = "";
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                String tmillis = String( millis() );
                while ( tmillis.length() < 10 )
                    tmillis = "0" + tmillis;
                msg.replace( "\n", "␤" );
                Serial.println( tmillis + ":" + entName + ": origin='" + origin + "' topic='" +
                                topic + "' body='" + msg + "'" );
            }
#else
            messagespy( String name = "", String subscription = "" )
                : meisterwerk::core::entity( "" ) {
            }

            bool registerEntity( unsigned long minMicroSecs = 0, unsigned int priority = 3 ) {
                // never register but fake success
                return true;
            }
#endif
        };
    } // namespace util
} // namespace meisterwerk