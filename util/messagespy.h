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

#ifdef _MW_DEBUG
        class messagespy : public meisterwerk::core::entity {
            private:
            String filter;

            public:
            messagespy( String name = "spy", String filter = "*" )
                : meisterwerk::core::entity( name, 0, meisterwerk::core::PRIORITY_NORMAL ), filter{filter} {
                // will autoregister
            }

            void setup() override {
                meisterwerk::core::entity::setup();
                // subscribe messages to display
                subscribe( filter );
            }

            virtual void receive( const char *origin, const char *topic, const char *msg ) override {
                char   szBuffer[24];
                String s1( origin );
                String s2( topic );
                String s3( msg );

                sprintf( szBuffer, "%010ld:", millis() );
                s3.replace( "\n", "␤" );
                Serial.println( szBuffer + entName + ": origin='" + s1 + "' topic='" + s2 + "' body='" + s3 + "'" );
            }
        };
#else
        class messagespy {
            public:
            // fake entity. Will neither register nor do anything
            messagespy( String name = "", String subscription = "" ) {
            }
        };
#endif
    } // namespace util
} // namespace meisterwerk