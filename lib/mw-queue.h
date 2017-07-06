#ifndef _MW_QUEUE_H
#define _MW_QUEUE_H

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

template <class T>
class MW_Queue {
    private:
        T** que;
        unsigned int maxSize;
        unsigned int size;
        unsigned int quePtr0;
        unsigned int quePtr1;
    public:
    MW_Queue(unsigned int maxQueueSize) {
        quePtr0=0;
        quePtr1=0;
        size=0;
        maxSize=maxQueueSize;
        que=(T**)malloc(sizeof(T*)*maxSize);
    }
    bool push(T* ent) {
        if (size>=maxSize) return false;
        que[quePtr1]=ent;
        quePtr1=(quePtr1+1)%maxSize;
        ++size;
        return true;
    }
    T* pop() {
        if (size==0) return nullptr;
        T* pEnt=que[quePtr0];
        quePtr0=(quePtr0+1)%maxSize;
        --size;
        return pEnt;
    }
    bool isEmpty() {
        if (size==0) return true;
        else return false;
    }
    unsigned int length() {
        return (size);
    }
};

MW_Queue<T_MW_MSG> mw_msgQueue(MW_MAX_QUEUE);

#endif
