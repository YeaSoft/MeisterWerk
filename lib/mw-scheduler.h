#include <functional>
#include <map>

using std::function;

typedef function<void(unsigned long)> T_LOOPCALLBACK;

#define MW_PRIORITY_SYSTEMCRITICAL 0
#define MW_PRIORITY_TIMECRITICAL   1
#define MW_PRIORITY_HIGH           2
#define MW_PRIORITY_NORMAL         3
#define MW_PRIORITY_LOW            4
#define MW_PRIORITY_LOWEST         5

typedef struct t_task {
    T_LOOPCALLBACK loopcallback; // = function <void(unsigned long)> loopcallback; // = void (* loopcallback)(unsigned long);
    unsigned long minmicros;      // Intervall task should be called in microsecs.
    unsigned long lastcall;       // last microsec timestamp task was called
    unsigned long numberOfCalls;  // number of times, task has been executed
    unsigned long budget;         // Sum of microsecs used by this task during all calls
    unsigned long latetime;       // Sum of microsecs the task was scheduled later than requested by minmicros
    unsigned int priority;        // MW_PRIORITY_*
} T_TASK;

class MW_Entity {
    virtual ~MW_Entity() {}; // Otherwise destructor of derived classes is never called!

    bool sendToQueue() {
        // ... sends to scheduler-queue
    }
    bool registerEntity() {

    }
    virtual void setup()  { return; }
    virtual void loop(unsigned long ticker)  { return; }
    virtual void receiveCmds()  { return; }
};

class MW_Scheduler {
    private:
    // The task list;
    std::map<String, T_TASK *> mw_tasklist;

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
        unsigned long ticker=micros(); // XXX: handle ticker overflow!
        unsigned long schedulerdelta=ticker-lasttick;
        // XXX: sort tasks according to urgency
        for (auto t : mw_tasklist) {
            ticker=micros();
            T_TASK* ptask=t.second;
            unsigned long tdelta=ticker-ptask->lastcall;
            if (tdelta>=ptask->minmicros) {
                // Serial.println("Scheduling: "+t.first+" ticker: "+String(ticker)+" tdelta: "+String(tdelta));
                ptask->loopcallback(ticker);
                ptask->lastcall=ticker;
                ptask->budget+=micros()-ticker;
                ptask->latetime+=tdelta-ptask->minmicros;
                ++(ptask->numberOfCalls);
            }

        }
    }
    void addTask(String name, T_LOOPCALLBACK loopcallback, unsigned long minmicrosecs=0, unsigned int priority=1) {
        T_TASK* ptask=new T_TASK; // XXX: check
        ptask->loopcallback=loopcallback;
        ptask->minmicros=minmicrosecs;
        ptask->lastcall=0;
        ptask->numberOfCalls=0;
        ptask->budget=0;
        ptask->priority=priority;
        mw_tasklist[name]=ptask;
    }
};
