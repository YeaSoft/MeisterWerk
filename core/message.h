// message.h - The internal message class
//
// This is the declaration of the internal message
// class that is part of the implementation of the
// application method for non blocking communication
// between the components and scheduling

#ifndef message_h
#define message_h

// configuration of message properties
#ifndef MW_MSG_MAX_TOPIC_LENGTH
#define MW_MSG_MAX_TOPIC_LENGTH 32
#endif
#ifndef MW_MSG_MAX_MSGBUFFER_LENGTH
#define MW_MSG_MAX_MSGBUFFER_LENGTH 256
#endif

// configuration of the message Queue
#ifndef MW_MAX_QUEUE
#define MW_MAX_QUEUE 256
#endif

#include "helpers.h"
#include "queue.h"

namespace meisterwerk {
    namespace core {

        class message {
            public:
            static const unsigned int MSG_NONE      = 0;
            static const unsigned int MSG_DIRECT    = 1;
            static const unsigned int MSG_PUBLISH   = 2;
            static const unsigned int MSG_SUBSCRIBE = 3;

            // static operations
            static bool send( unsigned int _type, String _originator, String _topic, char *_pBuf,
                              unsigned int _len, bool isBufAllocated = false ) {
                message *msg = new message();
                if ( msg == nullptr ) {
                    DBG( "message::sendMessage, failed to allocate message" );
                    return false;
                }
                if ( msg->create( _type, _originator, _topic, _pBuf, _len, isBufAllocated ) ) {
                    messageQueue.push( msg );
                    return true;
                }
                return false;
            }

            static bool send( unsigned int _type, String _originator, String _topic,
                              String _content ) {
                if ( _content == nullptr || _content.length() == 0 ) {
                    return send( _type, _originator, _topic, nullptr, 0 );
                }
                unsigned int _length = _content.length() + 1;
                char *       _buffer = (char *)malloc( _length );
                if ( _buffer == nullptr ) {
                    DBG( "message::sendMessage, failed to allocate content" );
                    return false;
                }
                strcpy( _buffer, _content.c_str() );
                return send( _type, _originator, _topic, _buffer, _length );
            }

            // methods
            public:
            message() {
                init();
            }

            virtual ~message() {
                discard();
            }

            void init( unsigned int _type = 0, char *_originator = nullptr, char *_topic = nullptr,
                       char *_pBuf = nullptr, unsigned int _pBufLen = 0 ) {
                type       = _type;
                originator = _originator;
                topic      = _topic;
                pBuf       = _pBuf;
                pBufLen    = _pBufLen;
            }

            bool create( unsigned int _type, String _originator, String _topic, char *_pBuf,
                         unsigned int _len, bool isBufAllocated = false ) {
                DBG( "message::create, from: " + _originator + ", topic: " + _topic );
                int tLen = _topic.length() + 1;
                if ( tLen > MW_MSG_MAX_TOPIC_LENGTH || _len > MW_MSG_MAX_MSGBUFFER_LENGTH ) {
                    DBG( "message::create, size too large. " + _topic );
                    return false;
                }
                // free previous content if any
                discard();

                // set type
                type = _type;

                // set originator
                int olen   = _originator.length() + 1;
                originator = (char *)malloc( olen );
                if ( originator == nullptr ) {
                    DBG( "message::create, Cannot allocate originator" );
                    return false;
                }
                strcpy( originator, _originator.c_str() );

                // set topic
                topic = (char *)malloc( tLen );
                if ( topic == nullptr ) {
                    DBG( "message::create, Cannot allocate topic" );
                    discard();
                    return false;
                }
                strcpy( topic, _topic.c_str() );

                // set content
                if ( _len > 0 ) {
                    if ( isBufAllocated ) {
                        pBuf = _pBuf;
                    } else {
                        pBuf = (char *)malloc( _len );
                        if ( pBuf == nullptr ) {
                            DBG( "message::create, Cannot allocate content" );
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

            // static members
            static queue<message> messageQueue;

            // message members
            unsigned int type;       // MW_MSG_*
            unsigned int pBufLen;    // Length of binary buffer pBuf
            char *       originator; // allocated instance name of originator
            char *       topic;      // allocated zero terminated string
            char *       pBuf;       // allocated bin buffer of size pBufLen
        };

        // Instantiate the message queue
        queue<message> message::messageQueue( MW_MAX_QUEUE );
    }
}

#endif
