// queue.h - The internal entity class
//
// This is the declaration of the internal entity
// class that is part of the implementation of the
// application method for non blocking communication
// between the components and scheduling

#pragma once

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../util/debug.h"
#include "message.h"

namespace meisterwerk {
    namespace core {

        class entity;

        class msgregister {
            public:
            // members
            entity *      pEnt;         // instance object pointer to derived object instance
            unsigned int  priority;     // Priority MW_PRIORITY_*
            unsigned long minMicroSecs; // intervall in micro seconds the loop method
                                        // should be called. 0 means never (used for
                                        // messaging only entities)

            // methods
            msgregister( entity *pEnt, unsigned long minMicroSecs, unsigned int priority )
                : pEnt{pEnt}, minMicroSecs{minMicroSecs}, priority{priority} {
            }
        };

        class entity {
            public:
            // members
            String entName; // Instance name

            // methods
            entity( String name ) {
                entName = name;
            }

            virtual ~entity(){};

            bool registerEntity( unsigned long minMicroSecs = 0, unsigned int priority = 3 ) {
                msgregister reg( this, minMicroSecs, priority );
                if ( message::send( message::MSG_DIRECT, entName, "register", &reg, sizeof( reg ) ) ) {
                    return true;
                }
                DBG( "entity::registerEntity, sendMessage failed for register " + entName );
                return false;
            }

            bool publish( String topic, JsonObject &json ) const {
                String buffer;
                json.printTo( buffer );
                return publish( topic, buffer );
            }

            // Please keep in mind that you MUST attach the string representation
            // of a json object as msg.
            bool publish( String topic, String msg = "{}" ) const {
                if ( message::send( message::MSG_PUBLISH, entName, topic, msg ) ) {
                    return true;
                }
                DBG( "entity::publish, sendMessage failed for publish " + entName );
                return false;
            }

            bool subscribe( String topic ) const {
                if ( message::send( message::MSG_SUBSCRIBE, entName, topic, nullptr, 0 ) ) {
                    return true;
                }
                DBG( "entity::subscribe, sendMessage failed for subscribe " + entName );
                return false;
            }

            bool unsubscribe( String topic ) const {
                if ( message::send( message::MSG_UNSUBSCRIBE, entName, topic, nullptr, 0 ) ) {
                    return true;
                }
                DBG( "entity::unsubscribe, sendMessage failed for unsubscribe " + entName );
                return false;
            }

            String allTopic( String subtopic ) const {
                return "*/" + subtopic;
            }

            bool isAllTopic( String topic, String subtopic ) const {
                return topic == "*/" + subtopic;
            }

            String ownTopic( String subtopic ) const {
                return entName + "/" + subtopic;
            }

            bool isOwnTopic( String topic, String subtopic ) const {
                return topic == entName + "/" + subtopic;
            }

            // callbacks
            public:
            // When ou override this callback, do not forget to invoke the
            // base class implkementation that provides the mandatory
            // subscription to getstate and setstate
            virtual void onRegister() {
                // implementation not mandatory
                // mandatory subscriptions
                subscribe( ownTopic( "getstate" ) );
                subscribe( ownTopic( "setstate" ) );
            }

            // there is no clash between baseapp:onLoop and entity;:onLoop
            // because of a different argument list.
            virtual void onLoop( unsigned long ticker ) {
                // should be implemented if it is called - issue warning
                DBG( "entity:onLook, missing override for entity " + entName );
            }

            // Override this only if you have to implement your own message
            // handlers. BEWARE: Do not forget to invoke the base class
            // implementation that provides the mandatory handling of
            // getstate and setstate! Example:
            //
            // ...
            // if ( meisterwerk::core::entity::onReceive (...) ) {
            //     return true;
            // }
            // ...
            virtual bool onReceive( String origin, String topic, JsonObject &request, JsonObject &response ) {
                if ( isOwnTopic( topic, "getstate" ) ) {
                    onGetState( request, response );
                    publish( ownTopic( "state" ), response );
                    return true;
                } else if ( isOwnTopic( topic, "setstate" ) ) {
                    if ( onSetState( request, response ) ) {
                        publish( ownTopic( "state" ), response );
                    }
                    return true;
                }
                return false;
            }

            // ABSTRACT METHODS - Must be implemented in derived classes
            virtual void onGetState( JsonObject &request, JsonObject &response ) = 0;
            virtual bool onSetState( JsonObject &request, JsonObject &response ) = 0;

            // utilities
            static bool willSetStateB( JsonVariant &test, bool value ) {
                return test.success() && value != test.as<bool>();
            }
            static bool willSetStateU( JsonVariant &test, unsigned long value ) {
                return test.success() && value != test.as<unsigned long>();
            }

            static bool willSetStateS( JsonVariant &test, String value ) {
                return test.success() && value != test.as<String>();
            }

            // The following function is invoked ONLY by the schduler!
            virtual void processMessage( String origin, String topic, String msg ) {
                DynamicJsonBuffer reqBuffer( 300 );
                DynamicJsonBuffer resBuffer( 300 );
                JsonObject &      req         = reqBuffer.parseObject( msg );
                JsonObject &      res         = resBuffer.createObject();
                const char *      correlation = req["correlation"];
                if ( correlation ) {
                    res["correlation"] = correlation;
                }
                onReceive( origin, topic, req, res );
            }
        };
    } // namespace core
} // namespace meisterwerk
