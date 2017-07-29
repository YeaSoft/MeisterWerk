// queue.h - The internal entity class
//
// This is the declaration of the internal entity
// class that is part of the implementation of the
// application method for non blocking communication
// between the components and scheduling

#pragma once

// dependencies
#include "../util/debug.h"
#include "../util/msgtime.h"
#include "common.h"
#include "message.h"

namespace meisterwerk {
    namespace core {

        class entity;

        class msgregister {
            public:
            // members
            entity *      pEnt;         // instance object pointer to derived object instance
            T_PRIO        priority;     // Priority MW_PRIORITY_*
            unsigned long minMicroSecs; // intervall in micro seconds the loop method
                                        // should be called. 0 means never (used for
                                        // messaging only entities)

            // methods
            msgregister( entity *pEnt, unsigned long minMicroSecs, T_PRIO priority )
                : pEnt{pEnt}, minMicroSecs{minMicroSecs}, priority{priority} {
            }
        };

        class entity {
            public:
            // members
            String entName; // Instance name

            // methods

            // constructor without autoregistration
            entity( String name ) : entName( name ) {
            }

            // EXPERIMENTAL: constructor with autoregistration
            entity( String name, unsigned long minMicroSecs, T_PRIO priority = PRIORITY_NORMAL ) : entName( name ) {
                registerEntity( minMicroSecs, priority );
            }

            virtual ~entity(){};

            bool registerEntity( unsigned long minMicroSecs = 0, T_PRIO priority = PRIORITY_NORMAL ) {
                msgregister reg( this, minMicroSecs, priority );
                if ( message::send( message::MSG_DIRECT, entName.c_str(), "register", &reg, sizeof( reg ) ) ) {
                    return true;
                }
                DBG( "entity::registerEntity, sendMessage failed for register " + entName );
                return false;
            }

            bool updateEntity( unsigned long minMicroSecs = 0, T_PRIO priority = PRIORITY_NORMAL ) {
                msgregister reg( this, minMicroSecs, priority );
                if ( message::send( message::MSG_DIRECT, entName.c_str(), "update", &reg, sizeof( reg ) ) ) {
                    return true;
                }
                DBG( "entity::updateEntity, sendMessage failed for update " + entName );
                return false;
            }

            bool publish( String topic, String msg ) const {
                if ( message::send( message::MSG_PUBLISH, entName.c_str(), topic.c_str(), msg.c_str() ) ) {
                    return true;
                }
                DBG( "entity::publish, sendMessage failed for publish " + entName );
                return false;
            }

            bool publish( String topic ) const {
                if ( message::send( message::MSG_PUBLISH, entName.c_str(), topic.c_str(), nullptr ) ) {
                    return true;
                }
                DBG( "entity::publish, sendMessage failed for publish " + entName );
                return false;
            }

            bool subscribe( String topic ) const {
                if ( message::send( message::MSG_SUBSCRIBE, entName.c_str(), topic.c_str(), nullptr, 0 ) ) {
                    return true;
                }
                DBG( "entity::subscribe, sendMessage failed for subscribe " + entName );
                return false;
            }

            bool unsubscribe( String topic ) const {
                if ( message::send( message::MSG_UNSUBSCRIBE, entName.c_str(), topic.c_str(), nullptr, 0 ) ) {
                    return true;
                }
                DBG( "entity::unsubscribe, sendMessage failed for unsubscribe " + entName );
                return false;
            }

            // possible preliminary home of log functions
            enum loglevel { ERR, WARN, INFO, DBG, VER1, VER2, VER3 };
            loglevel logLevel = loglevel::INFO;
            void setLogLevel( loglevel lclass ) {
                logLevel = lclass;
            }
            void log( loglevel lclass, String msg, String logtopic = "" ) {
                if ( lclass > logLevel )
                    return;
                String cstr;
                switch ( lclass ) {
                case loglevel::ERR:
                    cstr = "Error";
                    break;
                case loglevel::WARN:
                    cstr = "Warning";
                    break;
                case loglevel::INFO:
                    cstr = "Info";
                    break;
                case loglevel::DBG:
                    cstr = "Debug";
                    break;
                default:
                    cstr = "Debug";
                    break;
                }
                publish( "log/" + cstr + "/" + entName, "{\"time\":\"" + util::msgtime::ISOnowMillis() +
                                                                "\",\"severity\":\"" + cstr + "\",\"topic\":\"" +
                                                                ( logtopic == "" )
                                                            ? entName
                                                            : logtopic + "\",\"msg\":\"" + msg + "\"}" );
            }

            // callbacks
            public:
            virtual void setup() {
            }

            virtual void loop() {
            }

            virtual void receive( const char *origin, const char *topic, const char *msg ) {
            }
        };
    } // namespace core
} // namespace meisterwerk
