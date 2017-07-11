// queue.h - The internal entity class
//
// This is the declaration of the internal entity
// class that is part of the implementation of the
// application method for non blocking communication
// between the components and scheduling

#pragma once

// dependencies
#include "helpers.h"
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
            msgregister( entity *_pEnt, unsigned long _minMicroSecs, unsigned int _priority ) {
                pEnt         = _pEnt;
                priority     = _priority;
                minMicroSecs = _minMicroSecs;
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
                if ( sendMessage( message::MSG_DIRECT, "register", (char *)&reg, sizeof( reg ) ) ) {
                    return true;
                }
                DBG( "entity::registerEntity, sendMessage failed for register " + entName );
                return false;
            }

            bool publish( String topic, String msg ) {
                if ( sendMessage( message::MSG_PUBLISH, topic, msg ) ) {
                    return true;
                }
                DBG( "entity::publish, sendMessage failed for publish " + entName );
                return false;
            }

            bool subscribe( String topic ) {
                if ( sendMessage( message::MSG_SUBSCRIBE, topic, nullptr, 0 ) ) {
                    return true;
                }
                DBG( "entity::publish, sendMessage failed for subscribe " + entName );
                return false;
            }

            virtual void onSetup() {
                DBG( "entity::onSetup, missing override for entity " + entName );
            }

            virtual void onLoop( unsigned long ticker ) {
                DBG( "entity:onLook, missing override for entity " + entName );
            }

            virtual void onReceiveMessage( String topic, const char *pBuf, unsigned int len ) {
                DBG( "entity:receiveMessage, missing override for entity " + entName );
            }

            protected:
            // This sends messages to scheduler via messageQueue
            bool sendMessage( unsigned int type, String topic, char *pBuf, unsigned int len,
                              bool isBufAllocated = false ) {
                // DBG( "entity::sendMessage, from: " + entName + ", topic: " + topic );
                return message::send( type, entName, topic, pBuf, len, isBufAllocated );
            }

            // Send text message to Scheduler
            bool sendMessage( unsigned int type, String topic, String content ) {
                // DBG( "entity::sendMessage, from: " + entName + ", topic: " + topic );
                return message::send( type, entName, topic, content );
            }
        };
    } // namespace core
} // namespace meisterwerk
