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
#else
            messagespy( String name = "", String subscription = "" )
                : meisterwerk::core::entity( "" ) {
            }

            bool registerEntity( unsigned long minMicroSecs = 0, unsigned int priority = 3 ) {
                // never register
                return true;
            }
#endif

#ifdef _DEBUG
            void onRegister() override {
                subscribe( tmpSubscribedTopic );
                tmpSubscribedTopic = "";
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                Serial.println( "messagespy(" + entName + "): origin='" + origin + "' topic='" +
                                topic + "' body='" + msg + "'" );
            }
#endif
        };
    } // namespace util
} // namespace meisterwerk