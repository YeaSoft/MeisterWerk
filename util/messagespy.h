// messagespy.h - A helper entity for message monitoring
//
// This is the declaration of a monitoring class for
// internal message monitoring. The messages are
// emitted through the serial interface
// The class works only if #define DEBUG

// dependencies
#include "../core/entity.h"

namespace meisterwerk {
    namespace util {

        class messagespy : public meisterwerk::core::entity {
            private:
#ifdef DEBUG
            String tmpSubscribedTopic;
#endif

            public:
#ifdef DEBUG
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

#ifdef DEBUG
            void onSetup() override {
                subscribe( tmpSubscribedTopic );
                tmpSubscribedTopic = "";
            }

            virtual void onReceiveMessage( String topic, const char *pBuf, unsigned int len ) {
                if ( len = 0 ) {
                    Serial.println( "messagespy(" + entName + "): topic='" + topic + "'" );
                } else {
                    Serial.println( "messagespy(" + entName + "): topic='" + topic + "' body='" +
                                    String( pBuf ) + "'" );
                }
            }
#endif
        };
    }
}