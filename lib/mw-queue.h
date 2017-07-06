#ifndef _MW_QUEUE_H
#define _MW_QUEUE_H

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

#endif
