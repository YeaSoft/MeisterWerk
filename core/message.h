// message.h - The internal message class
//
// This is the declaration of the internal message
// class that is part of the implementation of the
// application method for non blocking communication
// between the components and scheduling

#pragma once

// configuration of message properties
#ifndef MW_MSG_MAX_TOPIC_LENGTH
#define MW_MSG_MAX_TOPIC_LENGTH 32
#endif
#ifndef MW_MSG_MAX_MSGBUFFER_LENGTH
#define MW_MSG_MAX_MSGBUFFER_LENGTH 768
#endif

// configuration of the message Queue
#ifndef MW_MAX_QUEUE
#define MW_MAX_QUEUE 256
#endif

// dependencies
#include "../util/debug.h"
#include "queue.h"

namespace meisterwerk {
    namespace core {

        class message {
            public:
            // constants
            static const unsigned int MSG_NONE        = 0;
            static const unsigned int MSG_DIRECT      = 1;
            static const unsigned int MSG_SUBSCRIBE   = 2;
            static const unsigned int MSG_UNSUBSCRIBE = 3;
            static const unsigned int MSG_PUBLISH     = 4;
            static const unsigned int MSG_PUBLISHRAW  = 5;

            // static members
            static queue<message> que;

            // message members
            unsigned int type;       // MW_MSG_*
            unsigned int pBufLen;    // Length of binary buffer pBuf
            char *       originator; // allocated instance name of originator
            char *       topic;      // allocated zero terminated string
            void *       pBuf;       // allocated bin buffer of size pBufLen

            // static methods
            static bool send( unsigned int _type, const char *_originator, const char *_topic, const void *_pBuf,
                              unsigned int _len, bool isBufAllocated = false ) {
                message *msg = new message();
                if ( msg == nullptr ) {
                    DBG( F( "message::sendMessage, failed to allocate message" ) );
                    return false;
                }
                if ( msg->create( _type, _originator, _topic, _pBuf, _len, isBufAllocated ) ) {
                    que.push( msg );
                    return true;
                }
                return false;
            }

            static bool send( unsigned int _type, const char *_originator, const char *_topic, const char *_content ) {
                if ( _content == nullptr || strlen( _content ) == 0 ) {
                    return send( _type, _originator, _topic, nullptr, 0 );
                }
                size_t _length = strlen( _content ) + 1;
                char * _buffer = (char *)malloc( _length );
                if ( _buffer == nullptr ) {
                    DBG( F( "message::sendMessage, failed to allocate content" ) );
                    return false;
                }
                strcpy( _buffer, _content );
                return send( _type, _originator, _topic, _buffer, _length, true );
            }

            // methods
            message() {
                init();
            }

            virtual ~message() {
                discard();
            }

            void init( unsigned int _type = 0, const char *_originator = nullptr, const char *_topic = nullptr,
                       const void *_pBuf = nullptr, unsigned int _pBufLen = 0 ) {
                type       = _type;
                originator = (char *)_originator;
                topic      = (char *)_topic;
                pBuf       = (void *)_pBuf;
                pBufLen    = _pBufLen;
            }

            bool create( unsigned int _type, const char *_originator, const char *_topic, const void *_pBuf,
                         unsigned int _len, bool isBufAllocated = false ) {
                if ( _originator == nullptr || _topic == nullptr ) {
                    DBG( "message::create, originator and topic must be speicifed." );
                    return false;
                }

                size_t tLen = strlen( _topic ) + 1;
                if ( tLen > MW_MSG_MAX_TOPIC_LENGTH || _len > MW_MSG_MAX_MSGBUFFER_LENGTH ) {
                    DBG( "message::create, size too large. " + String( _topic ) );
                    return false;
                }
                // free previous content if any
                discard();

                // set type
                type = _type;

                // set originator
                size_t olen = strlen( _originator ) + 1;
                originator  = (char *)malloc( olen );
                if ( originator == nullptr ) {
                    DBG( F( "message::create, Cannot allocate originator" ) );
                    return false;
                }
                strcpy( originator, _originator );

                // set topic
                topic = (char *)malloc( tLen );
                if ( topic == nullptr ) {
                    DBG( F( "message::create, Cannot allocate topic" ) );
                    discard();
                    return false;
                }
                strcpy( topic, _topic );

                // set content
                if ( _len > 0 ) {
                    if ( isBufAllocated ) {
                        pBuf = (void *)_pBuf;
                    } else {
                        pBuf = (void *)malloc( _len );
                        if ( pBuf == nullptr ) {
                            DBG( F( "message::create, Cannot allocate content" ) );
                            discard();
                            return false;
                        }
                        memcpy( pBuf, _pBuf, _len );
                    }
                    pBufLen = _len;
                }
                return true;
            }

            void discard() {
                if ( pBuf != nullptr ) {
                    free( pBuf );
                }
                if ( topic != nullptr ) {
                    free( topic );
                }
                if ( originator != nullptr ) {
                    free( originator );
                }
                init();
            }
        };

        // Instantiate the message queue
        queue<message> message::que( MW_MAX_QUEUE );
    } // namespace core
} // namespace meisterwerk
