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
#ifdef _DEBUG
            private:
            String filter;

            public:
            messagespy( String name = "spy", String filter = "*" ) : meisterwerk::core::entity( name ), filter{filter} {
            }

            void onRegister() override {
                meisterwerk::core::entity::onRegister();
                // subscribe messages to display
                subscribe( filter );
            }

            /*
            virtual void onGetState( JsonObject &request, JsonObject &response ) override {
                response["type"]   = "messagespy";
                response["filter"] = filter;
            }

            virtual bool onSetState( JsonObject &request, JsonObject &response ) override {
                JsonVariant toFilter = request["filter"];
                if ( willSetStateS( toFilter, filter ) ) {
                    filter             = toFilter.as<String>();
                    response["filter"] = filter;
                    return true;
                }
                return false;
            }
            */

            virtual void processMessage( const char *origin, const char *topic, const char *msg ) override {
                char   szBuffer[24];
                String s1( origin );
                String s2( topic );
                String s3( msg );

                sprintf( szBuffer, "%010ld:", millis() );
                s3.replace( "\n", "␤" );
                Serial.println( szBuffer + entName + ": origin='" + s1 + "' topic='" + s2 + "' body='" + s3 + "'" );
                meisterwerk::core::entity::processMessage( origin, topic, msg );
            }

#else
            // fake entity. Will neither register nor do anything
            messagespy( String name = "", String subscription = "" ) : meisterwerk::core::entity( "" ) {
            }

            bool registerEntity( unsigned long minMicroSecs = 0, unsigned int priority = 3 ) {
                return true;
            }
#endif
        };
    } // namespace util
} // namespace meisterwerk