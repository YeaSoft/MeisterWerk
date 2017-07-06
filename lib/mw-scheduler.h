#ifndef _MW_SCHEDULER_H
#define _MW_SCHEDULER_H

#include <functional>
#include <map>

using std::function; // [OBSOLETE]

typedef function<void(unsigned long)> T_LOOPCALLBACK; // [OBSOLETE]

#define MW_PRIORITY_SYSTEMCRITICAL 0
#define MW_PRIORITY_TIMECRITICAL   1
#define MW_PRIORITY_HIGH           2
#define MW_PRIORITY_NORMAL         3
#define MW_PRIORITY_LOW            4
#define MW_PRIORITY_LOWEST         5

typedef struct t_task {
    MW_Entity* pEnt;               // Entity object
    T_LOOPCALLBACK loopCallback;  // [OBSOLETE] = function <void(unsigned long)> loopcallback; // = void (* loopcallback)(unsigned long);
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
    void loop() { 
        unsigned long ticker=micros(); // XXX: handle ticker overflow!
        unsigned long schedulerDelta=ticker-lastTick;
        // XXX: sort tasks according to urgency
        for (auto t : mw_taskList) {
            ticker=micros();
            T_TASK* pTask=t.second;
            unsigned long tDelta=ticker-pTask->lastCall;
            if (tDelta>=pTask->minMicros) {
                // Serial.println("Scheduling: "+t.first+" ticker: "+String(ticker)+" tdelta: "+String(tdelta));
                if (pTask->loopCallback != nullptr) // [OBSOLETE]
                    pTask->loopCallback(ticker); // Old callback style
                else {
                    if (pTask->pEnt != nullptr) { 
                        pTask->pEnt->loop(ticker); // new entity method
                    }
                }
                pTask->lastCall=ticker;
                pTask->budget+=micros()-ticker;
                pTask->lateTime+=tDelta-pTask->minMicros;
                ++(pTask->numberOfCalls);
            }

        }
    }

    // [OBSOLETE]?
    bool addTask(String name, T_LOOPCALLBACK loopCallback, unsigned long minMicroSecs=0, unsigned int priority=1) {
        T_TASK* pTask=new T_TASK;
        if (pTask==nullptr) return false;
        memset(pTask, 0, sizeof(T_TASK));
        pTask->pEnt=nullptr;
        pTask->loopCallback=loopCallback;
        pTask->minMicros=minMicroSecs;
        pTask->lastCall=0;
        pTask->numberOfCalls=0;
        pTask->budget=0;
        pTask->priority=priority;
        mw_taskList[name]=pTask;
        return true;
    }

    bool registerEntity(String name, MW_Entity* pEnt, unsigned long minMicroSecs=0, unsigned int priority=1) {
        T_TASK* pTask=new T_TASK;
        if (pTask==nullptr) return false;
        memset(pTask, 0, sizeof(T_TASK));
        pTask->pEnt=pEnt;
        pTask->loopCallback=nullptr;
        pTask->minMicros=minMicroSecs;
        pTask->lastCall=0;
        pTask->numberOfCalls=0;
        pTask->budget=0;
        pTask->priority=priority;
        mw_taskList[name]=pTask;
        return true;
    }
};

#endif
