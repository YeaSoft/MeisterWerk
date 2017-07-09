// queue.h - The internal queue class
//
// This is the declaration of the internal queue
// class that implements a basic queue mechanism
// for the internal non blocking communication
// between the components

#ifndef entity_h
#define entity_h

using std::function;

// states for buttons and leds
#define MW_STATE_OFF 0
#define MW_STATE_ON 1
#define MW_STATE_UNDEFINED 2

#define MW_MAX_QUEUE 256

#define MW_MSG_DIRECT 1
#define MW_MSG_PUBLISH 2
#define MW_MSG_SUBSCRIBE 3

typedef struct t_mw_msg {
    unsigned int type;       // MW_MSG_*
    unsigned int pBufLen;    // Length of binary buffer pBuf
    char *       originator; // allocated instance name of originator
    char *       topic;      // allocated zero terminated string
    char *       pBuf;       // allocated bin buffer of size pBufLen
} T_MW_MSG;

class MW_Entity;

#define MW_MSG_REG_MAXNAME 32

// Type declaration for virtual override member functions loop and
// receiveMessage of MW_Entity:
typedef void ( MW_Entity::*T_OLOOPCALLBACK )( unsigned long );
typedef void ( MW_Entity::*T_ORECVCALLBACK )( String, char *pBuf,
                                              unsigned int len );
// Type declaration for static tasks:
typedef function<void( unsigned long )> T_LOOPCALLBACK;

typedef struct t_mw_msg_register {
    MW_Entity *     pEnt;  // instance object pointer to derived object instance
    T_OLOOPCALLBACK pLoop; // pointer to virtual override loop
    T_ORECVCALLBACK pRecv; // pointer to virtual override recvMessage
    char            entName[MW_MSG_REG_MAXNAME]; // Entity instance name
    unsigned long   minMicroSecs; // intervall in micro seconds the loop method
                                  // should be called
    unsigned int priority;        // Priority MW_PRIORITY_*
} T_MW_MSG_REGISTER;

MW_Queue<T_MW_MSG> mw_msgQueue( MW_MAX_QUEUE );

#define MW_MSG_MAX_TOPIC_LENGTH 32
#define MW_MSG_MAX_MSGBUFFER_LENGTH 256

class MW_Entity {
    private:
    // This sends messages to scheduler via mw_msgQueue
    bool sendMessage( int type, String topic, char *pBuf, int len,
                      bool isBufAllocated = false ) {
        Serial.println( "begin sendMessage, from: " + entName +
                        ", topic: " + topic );
        int tLen = topic.length() + 1;
        if ( tLen > MW_MSG_MAX_TOPIC_LENGTH ||
             len > MW_MSG_MAX_MSGBUFFER_LENGTH ) {
            Serial.println( "Msg discard, size too large. " + topic );
            return false;
        }
        T_MW_MSG *pMsg = (T_MW_MSG *)malloc( sizeof( T_MW_MSG ) );
        if ( pMsg == nullptr )
            return false;
        pMsg->type  = type;
        pMsg->topic = (char *)malloc( tLen );
        if ( pMsg->topic == nullptr ) {
            free( pMsg );
            return false;
        }
        strcpy( pMsg->topic, topic.c_str() );

        if ( len == 0 ) {
            pMsg->pBuf    = nullptr;
            pMsg->pBufLen = 0;
        } else {
            if ( isBufAllocated ) {
                pMsg->pBuf = pBuf;
            } else {
                pMsg->pBuf = (char *)malloc( len );
                if ( pMsg->pBuf == nullptr ) {
                    free( pMsg->topic );
                    free( pMsg );
                    return false;
                }
                memcpy( pMsg->pBuf, pBuf, len );
            }
            pMsg->pBufLen = len;
        }

        unsigned int olen = entName.length() + 1;
        pMsg->originator  = (char *)malloc( olen );
        strcpy( pMsg->originator, entName.c_str() );

        mw_msgQueue.push( pMsg );
        return true;
    }

    // Send text message to Scheduler
    bool sendMessage( int type, String topic, String content ) {
        if ( content == nullptr || content.length() == 0 ) {
            return sendMessage( type, topic, nullptr, 0 );
        }
        int   len  = content.length() + 1;
        char *pBuf = (char *)malloc( len );
        if ( pBuf == nullptr )
            return false;
        strcpy( pBuf, content.c_str() );
        return sendMessage( type, topic, pBuf, len );
    }

    public:
    String entName; // Instance name

    virtual ~MW_Entity(){}; // Otherwise destructor of derived classes is never
                            // called!

    bool registerEntity( String eName, MW_Entity *pen, T_OLOOPCALLBACK pLoop,
                         T_ORECVCALLBACK pRecv, unsigned long minMicroSecs = 0,
                         unsigned int priority = 1 ) {
        T_MW_MSG_REGISTER mr;
        if ( eName.length() >= MW_MSG_REG_MAXNAME - 1 ) {
            Serial.println( "Name to long for registration: " + eName );
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
            sendMessage( MW_MSG_DIRECT, "register", (char *)&mr, sizeof( mr ) );
        if ( !ret )
            Serial.println( "sendMessage failed for register " + eName );
        return ret;
    }

    bool publish( String topic, String msg ) {
        sendMessage( MW_MSG_PUBLISH, topic, msg );
        return false;
    }
    bool subscribe( String topic ) {
        sendMessage( MW_MSG_SUBSCRIBE, topic, nullptr, 0 );
        return false;
    }

    virtual void loop( unsigned long ticker ) {
        Serial.println( "Loop: class for " + entName +
                        " doesnt implement override! Wrong instance!" );
        return;
    }

    virtual void receiveMessage( String topic, char *pBuf, unsigned int len ) {
        Serial.println( "receiveMessage: class for " + entName +
                        " doesnt implement override! Wrong instance!" );
        return;
    }
};

#endif
