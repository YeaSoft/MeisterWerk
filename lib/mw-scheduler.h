#ifndef _MW_SCHEDULER_H
#define _MW_SCHEDULER_H

#include <map>

#define MW_PRIORITY_SYSTEMCRITICAL 0
#define MW_PRIORITY_TIMECRITICAL   1
#define MW_PRIORITY_HIGH           2
#define MW_PRIORITY_NORMAL         3
#define MW_PRIORITY_LOW            4
#define MW_PRIORITY_LOWEST         5

typedef struct t_task {
    // For static tasks:
    T_LOOPCALLBACK loopCallback;  // Static task function, function <void(unsigned long)> loopcallback; // = void (* loopcallback)(unsigned long);
    // For MW_Entity derived objects: (note: virtual methods have different pointer sizes than static function pointers!)
    MW_Entity* pEnt;              // pointer to derived instance of MW_Entity
    T_OLOOPCALLBACK oloopCallback;  // loop virtual override, = void (MW_Entity::* loopcallback)(unsigned long);
    T_ORECVCALLBACK orecvCallback;  // receiveMessage virtual override, receives incoming messages.
    // Common:
    unsigned long minMicros;      // Intervall task should be called in microsecs.
    unsigned long lastCall;       // last microsec timestamp task was called
    unsigned long numberOfCalls;  // number of times, task has been executed
    unsigned long budget;         // Sum of microsecs used by this task during all calls
    unsigned long lateTime;       // Sum of microsecs the task was scheduled later than requested by minmicros
    unsigned int priority;        // MW_PRIORITY_*
} T_TASK;

class MW_Scheduler {
    private:
    // The task list;
    std::map<String, T_TASK *> mw_taskList;

    bool bDebug;
    unsigned long lastTick;
    public:
    MW_Scheduler(bool bDebugMsg=false) {
        mw_taskList.clear();
        bDebug=bDebugMsg;
        lastTick=micros();
    }
    ~MW_Scheduler() {
        for (auto t : mw_taskList) {
            delete t.second;
        }
        mw_taskList.clear();
    }
    void discardMsg(T_MW_MSG *pMsg) {
        if (pMsg!=nullptr) {
            if (pMsg->pBuf!=nullptr) {
                free(pMsg->pBuf);
            }
            if (pMsg->topic!=nullptr) {
                free(pMsg->topic);
            }
            free(pMsg);
        }
    }
    void directMsg(T_MW_MSG *pMsg) {
        if (String(pMsg->topic)=="register") {
            if (pMsg->pBufLen!=sizeof(T_MW_MSG_REGISTER)) {
                Serial.println("Direct message: invalid message buffer size!"+String(pMsg->topic));            
            } else {
                T_MW_MSG_REGISTER *pReg=(T_MW_MSG_REGISTER *)pMsg->pBuf;
                registerEntity(String(pReg->entName), pReg->pEnt, pReg->pLoop, pReg->pRecv, pReg->minMicroSecs, pReg->priority);
                Serial.println("Registered entity: "+String(pReg->entName));
            }
        } else {
            Serial.println("Direct message: not implemented"+String(pMsg->topic));
        }
        discardMsg(pMsg);
    }
    void publishMsg(T_MW_MSG *pMsg) {
        Serial.println("Publish message: not implemented"+String(pMsg->topic));
        discardMsg(pMsg);
    }
    void subscribeMsg(T_MW_MSG *pMsg) {
        Serial.println("Subscribe message: not implemented"+String(pMsg->topic));
        discardMsg(pMsg);
    }
    void checkMsgQueue() {
        T_MW_MSG *pMsg;
        while (!mw_msgQueue.isEmpty()) {
            pMsg=mw_msgQueue.pop();
            if (pMsg!=nullptr) {
                switch (pMsg->type) {
                    case MW_MSG_DIRECT:
                        directMsg(pMsg);
                    break;
                    case MW_MSG_PUBLISH:
                        publishMsg(pMsg);
                    break;
                    case MW_MSG_SUBSCRIBE:
                        subscribeMsg(pMsg);
                    break;
                    default:
                        Serial.println("Unexpected message type: "+String(pMsg->type));
                        discardMsg(pMsg);
                    break;
                }
            }
        }
    }
    void loop() { 
        unsigned long ticker=micros(); // XXX: handle ticker overflow!
        unsigned long schedulerDelta=ticker-lastTick;
        // XXX: sort tasks according to urgency
        for (auto t : mw_taskList) {
            checkMsgQueue();
            ticker=micros();
            T_TASK* pTask=t.second;
            unsigned long tDelta=ticker-pTask->lastCall;
            if (tDelta>=pTask->minMicros) {
                // SLOW: Serial.println("Scheduling: "+t.first+" ticker: "+String(ticker)+" tdelta: "+String(tdelta));
                if (pTask->loopCallback != nullptr) {
                    pTask->loopCallback(ticker);
                } else {
                    if (pTask->oloopCallback != nullptr && pTask->pEnt != nullptr) {
                        // Serial.println(t.first+" going to be called! Nr: "+String(pTask->numberOfCalls));
                        // This nasty bugger calls the class-instances virtual override member loop:
                        ((pTask->pEnt)->*(pTask->oloopCallback))(ticker);
                        // Serial.println("DONE");
                    }   
                }
                pTask->lastCall=ticker;
                pTask->budget+=micros()-ticker;
                pTask->lateTime+=tDelta-pTask->minMicros;
                ++(pTask->numberOfCalls);
            }

        }
    }

    // This adds tasks as static functions:
    bool addTask(String eName, T_LOOPCALLBACK loopCallback, unsigned long minMicroSecs=0, unsigned int priority=1) {
        T_TASK* pTask=new T_TASK;
        if (pTask==nullptr) return false;
        memset(pTask, 0, sizeof(T_TASK));
        pTask->loopCallback=loopCallback;
        pTask->oloopCallback=nullptr;
        pTask->orecvCallback=nullptr;
        pTask->minMicros=minMicroSecs;
        pTask->lastCall=0;
        pTask->numberOfCalls=0;
        pTask->budget=0;
        pTask->priority=priority;
        mw_taskList[eName]=pTask;
        return true;
    }

    // This supports derived objects from MW_Entity: the scheduler accesses the virtual overrides for loop and receiveMessage:
    bool registerEntity(String eName, MW_Entity* pEnt, T_OLOOPCALLBACK loopCallback, T_ORECVCALLBACK recvCallback, unsigned long minMicroSecs=100000L, unsigned int priority=1) {
        T_TASK* pTask=new T_TASK;
        if (pTask==nullptr) return false;
        memset(pTask, 0, sizeof(T_TASK));
        pTask->pEnt=pEnt;
        pTask->loopCallback=nullptr;
        pTask->oloopCallback=loopCallback;
        pTask->orecvCallback=recvCallback;
        pTask->minMicros=minMicroSecs;
        pTask->lastCall=0;
        pTask->numberOfCalls=0;
        pTask->budget=0;
        pTask->priority=priority;
        mw_taskList[eName]=pTask;
        return true;
    }
};

#endif
