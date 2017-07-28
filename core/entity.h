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
                if ( message::send( message::MSG_DIRECT, entName.c_str(), "register", &reg, sizeof( reg ) ) ) {
                    return true;
                }
                DBG( "entity::registerEntity, sendMessage failed for register " + entName );
                return false;
            }

            bool updateEntity( unsigned long minMicroSecs = 0, unsigned int priority = 3 ) {
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
            void     setLogLevel( loglevel lclass ) {
                logLevel = lclass;
            }
            void Log( loglevel lclass, String msg, String logtopic = "" ) {
                if ( lclass > logLevel )
                    return;
                if ( logtopic == "" )
                    logtopic = entName;
                String cstr = "";
                String icon = "";
                switch ( lclass ) {
                case loglevel::ERR:
                    cstr = "Error";
                    icon = "🛑";
                    break;
                case loglevel::WARN:
                    cstr = "Warning";
                    icon = "⚠️";
                    break;
                case loglevel::INFO:
                    cstr = "Info";
                    icon = "ℹ️ℹ";
                    break;
                case loglevel::DBG:
                    cstr = "Debug";
                    icon = "🗒";
                    break;
                default:
                    cstr = "Debug";
                    icon = "🐞";
                    break;
                }
                // XXX: Json-in-message escaping, is that the best way:
                msg.replace( "\\", "/" );
                msg.replace( "\"", "'" );

                String tpc    = "log/" + cstr + "/" + entName;
                String logmsg = "{\"time\":\"" + util::msgtime::ISOnowMillis() + "\",\"severity\":\"" + cstr +
                                "\",\"icon\":\"" + icon + "\",\"topic\":\"" + logtopic + "\",\"msg\":\"" + msg + "\"}";
                publish( tpc, logmsg );
                DBG( icon + "  " + tpc + " | " + logmsg );
            }

            // callbacks
            public:
            virtual void onRegister() {
            }

            // there is no clash between baseapp:onLoop and entity;:onLoop
            // because of a different argument list.
            virtual void onLoop( unsigned long ticker ) {
                // should be implemented if it is called - issue warning
                DBG( "entity:onLook, missing override for entity " + entName );
            }

            virtual void onReceive( const char *origin, const char *topic, const char *msg ) {
                // should be implemented if it is called - issue warning:
                // XXX: we should make this optional, and
                // possibly provide standard functions (eg. set loglevel)
                DBG( "entity:onReceive, missing override for entity " + entName );
            }
        };
    } // namespace core
} // namespace meisterwerk
