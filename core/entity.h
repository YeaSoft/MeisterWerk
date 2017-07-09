// queue.h - The internal entity class
//
// This is the declaration of the internal entity
// class that is part of the implementation of the
// application method for non blocking communication
// between the components and scheduling

#ifndef entity_h
#define entity_h

// states for buttons and leds
#define MW_STATE_OFF 0
#define MW_STATE_ON 1
#define MW_STATE_UNDEFINED 2

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

        // Type declaration for virtual override member functions onLoop and
        // onReceiveMessage of entity:
        typedef void ( entity::*T_OLOOPCALLBACK )( unsigned long );
        typedef void ( entity::*T_ORECVCALLBACK )( String, char *pBuf, unsigned int len );
        // Type declaration for static tasks:
        typedef function<void( unsigned long )> T_LOOPCALLBACK;

        typedef struct t_mw_msg_register {
            entity *        pEnt;  // instance object pointer to derived object instance
            T_OLOOPCALLBACK pLoop; // pointer to virtual override loop
            T_ORECVCALLBACK pRecv; // pointer to virtual override recvMessage
            char            entName[MW_MSG_REG_MAXNAME]; // Entity instance name
            unsigned long   minMicroSecs; // intervall in micro seconds the loop method
                                          // should be called
            unsigned int priority;        // Priority MW_PRIORITY_*
        } T_MW_MSG_REGISTER;

        class entity {
            protected:
            // This sends messages to scheduler via messageQueue
            bool sendMessage( unsigned int type, String topic, char *pBuf, unsigned int len,
                              bool isBufAllocated = false ) {
                DBG( "entity::sendMessage, from: " + entName + ", topic: " + topic );
                return message::sendMessage( type, entName, topic, pBuf, len, isBufAllocated );
            }

            // Send text message to Scheduler
            bool sendMessage( unsigned int type, String topic, String content ) {
                DBG( "entity::sendMessage, from: " + entName + ", topic: " + topic );
                return message::sendMessage( type, entName, topic, content );
            }

            public:
            virtual ~entity(){}; // Otherwise destructor of derived classes is never
                                 // called!

            bool registerEntity( String eName, entity *pen, T_OLOOPCALLBACK pLoop,
                                 T_ORECVCALLBACK pRecv, unsigned long minMicroSecs = 0,
                                 unsigned int priority = 1 ) {
                T_MW_MSG_REGISTER mr;
                if ( eName.length() >= MW_MSG_REG_MAXNAME - 1 ) {
                    DBG( "entity::registerEntity, Name to long for registration: " + eName );
                    return false;
                }
                memset( &mr, 0, sizeof( mr ) );
                mr.pEnt  = pen;
                mr.pLoop = pLoop;
                mr.pRecv = pRecv;
                strcpy( mr.entName, eName.c_str() );
                mr.minMicroSecs = minMicroSecs;
                mr.priority     = priority;
                bool ret =
                    sendMessage( message::MSG_DIRECT, "register", (char *)&mr, sizeof( mr ) );
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

            // members
            public:
            String entName; // Instance name
        };
    }
}
#endif
