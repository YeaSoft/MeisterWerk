// queue.h - The internal entity class
//
// This is the declaration of the internal entity
// class that is part of the implementation of the
// application method for non blocking communication
// between the components and scheduling

#pragma once

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
                if ( message::send( message::MSG_DIRECT, entName, "register", &reg,
                                    sizeof( reg ) ) ) {
                    return true;
                }
                DBG( "entity::registerEntity, sendMessage failed for register " + entName );
                return false;
            }

            bool publish( String topic ) {
                if ( message::send( message::MSG_PUBLISH, entName, topic, "{}" ) ) {
                    return true;
                }
                DBG( "entity::publish, sendMessage failed for publish " + entName );
                return false;
            }

            bool publish( String topic, String msg ) {
                if ( message::send( message::MSG_PUBLISH, entName, topic, msg ) ) {
                    return true;
                }
                DBG( "entity::publish, sendMessage failed for publish " + entName );
                return false;
            }

            /* Prepared but not supported in the current scheduler
            bool publish( String topic, const void *pBuf, unsigned long len ) {
                if ( message::send( message::MSG_PUBLISHRAW, entName, topic, pBuf, len ) ) {
                    return true;
                }
                DBG( "entity::publish, sendMessage failed for publish " + entName );
                return false;
            }
            */

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

            bool subscribeme( String subtopic ) const {
                return subscribe( entName + "/" + subtopic );
            }

            bool unsubscribeme( String subtopic ) const {
                return unsubscribe( entName + "/" + subtopic );
            }

            bool subscribeall( String subtopic ) const {
                return subscribe( "*/" + subtopic );
            }

            bool unsubscribeall( String subtopic ) const {
                return unsubscribe( "*/" + subtopic );
            }

            const char *getname() const {
                return entName.c_str();
            }

            // callbacks
            public:
            virtual void onRegister() {
                // implementation not mandatory
            }

            // there is no clash between baseapp:onLoop and entity;:onLoop
            // because of a different argument list.
            virtual void onLoop( unsigned long ticker ) {
                // should be implemented if it is called - issue warning
                DBG( "entity:onLook, missing override for entity " + entName );
            }

            virtual void onReceive( String origin, String topic, String msg ) {
                // should be implemented if it is called - issue warning
                DBG( "entity:onReceive(string), missing override for entity " + entName );
            }

            /* Prepared but not supported in the current scheduler
            virtual void onReceive( String origin, String topic, const void *pBuf, unsigned int len
            ) {
                // should be implemented if it is called - issue warning
                DBG( "entity:onReceive(binary), missing override for entity " + entName );
            }
            */
        };
    } // namespace core
} // namespace meisterwerk
