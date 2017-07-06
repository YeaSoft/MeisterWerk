#ifndef _MW_ENTITY_H
#define _MW_ENTITY_H

using std::function;

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

class MW_Entity;

#define MW_MSG_REG_MAXNAME  32

// = void (* loopcallback)(unsigned long);
typedef void (MW_Entity::*T_OLOOPCALLBACK)(unsigned long);
typedef void (MW_Entity::*T_ORECVCALLBACK)(String, char* pBuf, unsigned int len);
typedef function<void(unsigned long)> T_LOOPCALLBACK;
// typedef function<void(String, char* pBuf, int len)> T_RECVCALLBACK;


typedef struct t_mw_msg_register {
    MW_Entity* pEnt;
    T_OLOOPCALLBACK pLoop;
    char kaka[50];
    T_ORECVCALLBACK pRecv;
    char name[MW_MSG_REG_MAXNAME];
    unsigned long minMicroSecs;
    unsigned int priority;
} T_MW_MSG_REGISTER;

MW_Queue<T_MW_MSG> mw_msgQueue(MW_MAX_QUEUE);

#define MW_MSG_MAX_TOPIC_LENGTH 32
#define MW_MSG_MAX_MSGBUFFER_LENGTH 256    

class MW_Entity {
    private:
    bool sendMessage(int type, String topic, char *pBuf, int len, bool isBufAllocated=false) {
        Serial.println("begin sendMessage: "+topic);
        int tLen=topic.length()+1;
        if (tLen>MW_MSG_MAX_TOPIC_LENGTH || len>MW_MSG_MAX_MSGBUFFER_LENGTH) {
            Serial.println("Msg discard, size too large. "+topic);
            return false;
        }
        T_MW_MSG* pMsg = (T_MW_MSG *)malloc(sizeof(T_MW_MSG));
        if (pMsg==nullptr) return false;
        pMsg->type=type;
        pMsg->topic=(char *)malloc(tLen);
        if (pMsg->topic==nullptr) {
            free(pMsg);
            return false;
        }
        strcpy(pMsg->topic, topic.c_str());
        Serial.println("Topic set.");

        if (len==0) {
            pMsg->pBuf=nullptr;
            pMsg->pBufLen=0;
        } else {
            if (isBufAllocated) {
                pMsg->pBuf=pBuf;
            } else {
                pMsg->pBuf=(char *)malloc(len);
                if (pMsg->pBuf==nullptr) {
                    free(pMsg->topic);
                    free(pMsg);
                    return false;
                }
                memcpy(pMsg->pBuf, pBuf, len);
            }
            pMsg->pBufLen=len;
        }
        Serial.println("Enquing");
        mw_msgQueue.push(pMsg);
        return true;
    }

    bool sendMessage(int type, String topic, String content) {
        if (content==nullptr || content.length()==0) {
            return sendMessage(type, topic, nullptr, 0);
        }
        int len=content.length()+1;
        char *pBuf=(char *)malloc(len);
        if (pBuf==nullptr) return false;
        strcpy(pBuf,content.c_str());
        return sendMessage(type, topic, pBuf, len);
    }

    public:
    String name;

    virtual ~MW_Entity() {}; // Otherwise destructor of derived classes is never called!

    bool registerEntity(String name, MW_Entity* pen, T_OLOOPCALLBACK pLoop, T_ORECVCALLBACK pRecv, unsigned long minMicroSecs=0, unsigned int priority=1) {
        T_MW_MSG_REGISTER mr;
        if (name.length()>=MW_MSG_REG_MAXNAME-1) {
            Serial.println("Name to long for registration: "+name);
            return false;
        }
        memset(&mr,0,sizeof(mr));
        mr.pEnt=pen;
        mr.pLoop=pLoop;
        mr.pRecv=pRecv;
        strcpy(mr.name,name.c_str());
        mr.minMicroSecs=minMicroSecs;
        mr.priority=priority;
        bool ret=sendMessage(MW_MSG_DIRECT, "register", (char *)&mr, sizeof(mr));
        if (!ret) Serial.println("sendMessage failed for register "+name);
        return ret;
    }

    bool publish(String topic, char *pbuf, unsigned int size) {
        // ... sends to scheduler-queue
        return false;
    }
    bool subscribe(String topic) {
        return false;
    }

    virtual void loop(unsigned long ticker) {
        Serial.println("Loop: class for "+name+" doesnt implement override! Wrong instance!");
        return;
    }

    virtual void receiveMessage(String topic, char *pBuf, unsigned int len) {
        Serial.println("receiveMessage: class for "+name+" doesnt implement override! Wrong instance!");
        return;
    }

};

#endif
