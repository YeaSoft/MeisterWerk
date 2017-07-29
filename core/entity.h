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
        class baseapp;

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
            friend class baseapp;

            public:
            enum T_LOGLEVEL { ERR, WARN, INFO, DBG, VER1, VER2, VER3 };
            // members
            String     entName;                     // Instance name
            T_LOGLEVEL logLevel = T_LOGLEVEL::INFO; // Logging level

            // methods
            entity( String name, unsigned long minMicroSecs, T_PRIO priority = PRIORITY_NORMAL ) : entName( name ) {
                msgregister reg( this, minMicroSecs, priority );
                message::send( message::MSG_DIRECT, entName.c_str(), "register", &reg, sizeof( reg ) );
            }

            virtual ~entity(){};

            bool setSchedulerParams( unsigned long minMicroSecs = 0, T_PRIO priority = PRIORITY_NORMAL ) {
                msgregister reg( this, minMicroSecs, priority );
                if ( message::send( message::MSG_DIRECT, entName.c_str(), "update", &reg, sizeof( reg ) ) ) {
                    return true;
                }
                DBG( "entity::setSchedulerParams, sendMessage failed for " + entName );
                return false;
            }

            bool publish( String topic, String msg ) const {
                if ( message::send( message::MSG_PUBLISH, entName.c_str(), topic.c_str(), msg.c_str() ) ) {
                    return true;
                }
                DBG( "entity::publish, sendMessage failed for " + entName );
                return false;
            }

            bool publish( String topic ) const {
                if ( message::send( message::MSG_PUBLISH, entName.c_str(), topic.c_str(), nullptr ) ) {
                    return true;
                }
                DBG( "entity::publish, sendMessage failed for " + entName );
                return false;
            }

            bool subscribe( String topic ) const {
                if ( message::send( message::MSG_SUBSCRIBE, entName.c_str(), topic.c_str(), nullptr, 0 ) ) {
                    return true;
                }
                DBG( "entity::subscribe, sendMessage failed for " + entName );
                return false;
            }

            bool unsubscribe( String topic ) const {
                if ( message::send( message::MSG_UNSUBSCRIBE, entName.c_str(), topic.c_str(), nullptr, 0 ) ) {
                    return true;
                }
                DBG( "entity::unsubscribe, sendMessage failed for " + entName );
                return false;
            }

            // possible preliminary home of log functions
            void setLogLevel( T_LOGLEVEL lclass ) {
                logLevel = lclass;
            }

            void log( T_LOGLEVEL lclass, String msg, String logtopic = "" ) {
                if ( lclass > logLevel )
                    return;
                String cstr;
                switch ( lclass ) {
                case T_LOGLEVEL::ERR:
                    cstr = "Error";
                    break;
                case T_LOGLEVEL::WARN:
                    cstr = "Warning";
                    break;
                case T_LOGLEVEL::INFO:
                    cstr = "Info";
                    break;
                case T_LOGLEVEL::DBG:
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

            private:
            entity( String name ) : entName{name} {
                // special constructor only for baseapp
            }
        };
    } // namespace core
} // namespace meisterwerk
