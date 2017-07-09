// queue.h - The internal entity class
//
// This is the declaration of the internal entity
// class that is part of the implementation of the
// application method for non blocking communication
// between the components and scheduling

#ifndef entity_h
#define entity_h

// entity configuration
#ifndef MW_MSG_REG_MAXNAME
#define MW_MSG_REG_MAXNAME 32
#endif

#include "helpers.h"
#include "message.h"

using std::function;

namespace meisterwerk {
    namespace core {

        class entity;

        class msgregister {
            public:
            // Type declaration for virtual override member functions onLoop and
            // onReceiveMessage of entity:
            typedef void ( entity::*T_OLOOPCALLBACK )( unsigned long );
            typedef void ( entity::*T_ORECVCALLBACK )( String, char *pBuf, unsigned int len );
            // Type declaration for static tasks:
            typedef function<void( unsigned long )> T_LOOPCALLBACK;

            // methods
            msgregister() {
                pEnt         = nullptr;
                pLoop        = nullptr;
                pRecv        = nullptr;
                entName[0]   = 0;
                priority     = 0;
                minMicroSecs = 0;
            }

            bool init( String eName, entity *_pEnt, T_OLOOPCALLBACK _pLoop, T_ORECVCALLBACK _pRecv,
                       unsigned long _minMicroSecs = 0, unsigned int _priority = 1 ) {
                if ( eName.length() >= MW_MSG_REG_MAXNAME - 1 ) {
                    DBG( "entity::registerEntity, Name to long for registration: " + eName );
                    return false;
                }
                pEnt         = _pEnt;
                pLoop        = _pLoop;
                pRecv        = _pRecv;
                priority     = _priority;
                minMicroSecs = _minMicroSecs;
                strcpy( entName, eName.c_str() );
                return true;
            }

            // members
            entity *        pEnt;  // instance object pointer to derived object instance
            T_OLOOPCALLBACK pLoop; // pointer to virtual override loop
            T_ORECVCALLBACK pRecv; // pointer to virtual override recvMessage
            char            entName[MW_MSG_REG_MAXNAME]; // Entity instance name
            unsigned int    priority;                    // Priority MW_PRIORITY_*
            unsigned long   minMicroSecs; // intervall in micro seconds the loop method
                                          // should be called
        };

        class entity {
            public:
            virtual ~entity(){}; // Otherwise destructor of derived classes is never
                                 // called!

            public:
            bool registerEntity( unsigned long minMicroSecs = 0, unsigned int priority = 1 ) {
                msgregister reg;
                if ( !reg.init( entName, this, nullptr, nullptr, minMicroSecs, priority ) ) {
                    return false;
                }
                bool ret =
                    sendMessage( message::MSG_DIRECT, "register", (char *)&reg, sizeof( reg ) );
                if ( !ret ) {
                    DBG( "entity::registerEntity, sendMessage failed for register " + eName );
                }
                return ret;
            }

            bool registerEntity( String eName, entity *pen, msgregister::T_OLOOPCALLBACK pLoop,
                                 msgregister::T_ORECVCALLBACK pRecv, unsigned long minMicroSecs = 0,
                                 unsigned int priority = 1 ) {
                msgregister reg;
                if ( !reg.init( eName, pen, pLoop, pRecv, minMicroSecs, priority ) ) {
                    return false;
                }

                bool ret =
                    sendMessage( message::MSG_DIRECT, "register", (char *)&reg, sizeof( reg ) );
                if ( !ret ) {
                    DBG( "entity::registerEntity, sendMessage failed for register " + eName );
                }
                return ret;
            }

            bool publish( String topic, String msg ) {
                sendMessage( message::MSG_PUBLISH, topic, msg );
                return false;
            }
            bool subscribe( String topic ) {
                sendMessage( message::MSG_SUBSCRIBE, topic, nullptr, 0 );
                return false;
            }

            virtual void onLoop( unsigned long ticker ) {
                DBG( "entity:onLook, class for " + entName +
                     " doesnt implement override! Wrong instance!" );
            }

            virtual void onReceiveMessage( String topic, char *pBuf, unsigned int len ) {
                DBG( "entity:receiveMessage, class for " + entName +
                     " doesnt implement override! Wrong instance!" );
            }

            protected:
            // This sends messages to scheduler via messageQueue
            bool sendMessage( unsigned int type, String topic, char *pBuf, unsigned int len,
                              bool isBufAllocated = false ) {
                DBG( "entity::sendMessage, from: " + entName + ", topic: " + topic );
                return message::send( type, entName, topic, pBuf, len, isBufAllocated );
            }

            // Send text message to Scheduler
            bool sendMessage( unsigned int type, String topic, String content ) {
                DBG( "entity::sendMessage, from: " + entName + ", topic: " + topic );
                return message::send( type, entName, topic, content );
            }

            // members
            public:
            String entName; // Instance name
        };
    }
}
#endif
