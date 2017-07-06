#ifndef _MW_ENTITY_H
#define _MW_ENTITY_H

#define MW_MAX_QUEUE 256

#define MW_MSG_DIRECT       1
#define MW_MSG_PUBLISH      2
#define MW_MSG_SUBSCRIBE    3

typedef struct t_mw_msg {
    unsigned int type;      // MW_MSG_*
    unsigned int pBufLen;   // Length of binary buffer pBuf
    char *topic;            // zero terminated string
    char *pBuf;             // bin buffer of size pBufLen
} T_MW_MSG;

MW_Queue<T_MW_MSG> mw_msgQueue(MW_MAX_QUEUE);

#define MW_MSG_MAX_TOPIC_LENGTH 32
#define MW_MSG_MAX_MSGBUFFER_LENGTH 256


class MW_Entity {
    private:
    bool sendMessage(int type, String topic, char *pBuf, int len, bool isBufAllocated=false) {
        int tLen=topic.length()+1;
        if (tLen>MW_MSG_MAX_TOPIC_LENGTH || len>MW_MSG_MAX_MSGBUFFER_LENGTH) {
            return false;
        }
        T_MW_MSG* pMsg = new T_MW_MSG;
        if (pMsg==nullptr) return false;
        pMsg->type=type;
        pMsg->topic=(char *)malloc(tLen);
        if (pMsg->topic==nullptr) {
            delete pMsg;
            return false;
        }
        strcpy(pMsg->topic, topic.c_str());

        if (isBufAllocated) {
            pMsg->pBuf=pBuf;
        } else {
            pMsg->pBuf=(char *)malloc(len);
            if (pMsg->pBuf==nullptr) {
                free(pMsg->topic);
                delete pMsg;
                return false;
            }
            memcpy(pMsg->pBuf, pBuf, len);
        }
        pMsg->pBufLen=len;
        mw_msgQueue.push(pMsg);
        return true;
    }
    
    bool sendMessage(int type, String topic, String content) {
        return false;
    }
    public:
    String name();

    virtual ~MW_Entity() {}; // Otherwise destructor of derived classes is never called!

    bool registerEntity(String name, unsigned long minMicroSecs=0, unsigned int priority=1) {

    }

    bool publish(String topic, char *pbuf, unsigned int size) {
        // ... sends to scheduler-queue
    }
    bool subscribe(String topic) {

    }


    virtual void setup()  { return; }
    virtual void loop(unsigned long ticker)  { return; }
    virtual void receiveMessage()  { return; }
};

#endif
