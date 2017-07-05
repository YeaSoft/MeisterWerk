#include <functional>
#include <map>

using std::function;

typedef function<void(unsigned long)> T_LOOPCALLBACK;

typedef struct t_task {
    T_LOOPCALLBACK loopcallback; // = function <void(unsigned long)> loopcallback; // = void (* loopcallback)(unsigned long);
    unsigned long minmicros;      // Intervall task should be called in microsecs.
    unsigned long lastcall;       // last microsec timestamp task was called
    unsigned long numberOfCalls;  // number of times, task has been executed
    unsigned long budget;         // Sum of microsecs used by this task during all calls
    unsigned long latetime;       // Sum of microsecs the task was scheduled later than requested by minmicros
    unsigned int priority;        // 0: critical, 1: important, 2: kind of important, 3: sloppy, 4: now and then, ...
} T_TASK;

// The task list;
std::map<String, T_TASK *> mw_tasklist;

class MW_Scheduler {
    private:
    bool bDebug;
    unsigned long lasttick;
    public:
    MW_Scheduler(bool bDebugMsg=false) {
        mw_tasklist.clear();
        bDebug=bDebugMsg;
        lasttick=micros();
    }
    ~MW_Scheduler() {
        for (auto t : mw_tasklist) {
            delete t.second;
        }
        mw_tasklist.clear();
    }
    void loop() { 
        unsigned long ticker=micros();
        unsigned long schedulerdelta=ticker-lasttick;
        for (auto t : mw_tasklist) {
            ticker=micros();
            T_TASK* ptask=t.second;
            unsigned long tdelta=ticker-ptask->lastcall;
            if (tdelta>ptask->minmicros) {
                // Serial.println("Scheduling: "+t.first);
                ptask->loopcallback(ticker);
                ptask->lastcall=ticker;
                ptask->budget+=micros()-ticker;
                ptask->latetime+=tdelta-ptask->minmicros;
                ++(ptask->numberOfCalls);
            }

        }
    }
    void addTask(String name, T_LOOPCALLBACK loopcallback, unsigned long minmicrosecs=0, unsigned int priority=1) {
        T_TASK* ptask=new T_TASK;
        ptask->loopcallback=loopcallback;
        ptask->minmicros=minmicrosecs;
        ptask->lastcall=0;
        ptask->numberOfCalls=0;
        ptask->budget=0;
        ptask->priority=priority;
        mw_tasklist[name]=ptask;
    }
};
