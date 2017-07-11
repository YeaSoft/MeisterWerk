// scheduler.h - The internal queue class
//
// This is the declaration of the internal scheduler
// class that is part of the implementation of the
// application method for non blocking communication
// between the components and scheduling

#ifndef scheduler_h
#define scheduler_h

// dependencies
#include "entity.h"
#include "helpers.h"
#include <list>

namespace meisterwerk {
    namespace core {

        class scheduler {
            public:
            // constants
            static const unsigned int PRIORITY_SYSTEMCRITICAL = 0;
            static const unsigned int PRIORITY_TIMECRITICAL   = 1;
            static const unsigned int PRIORITY_HIGH           = 2;
            static const unsigned int PRIORITY_NORMAL         = 3;
            static const unsigned int PRIORITY_LOW            = 4;
            static const unsigned int PRIORITY_LOWEST         = 5;

            // internal types
            protected:
            typedef struct {
                String subscriber;
                String topic;
            } T_SUBSCRIPTION;

            class task {
                public:
                task( entity *_pEnt, unsigned long _minMicros, unsigned int _priority ) {
                    pEnt      = _pEnt;
                    minMicros = _minMicros;
                    priority  = _priority;
                    lastCall  = 0;
                    lateTime  = 0;
#ifdef DEBUG
                    numberOfCalls    = 0;
                    numberOfMessages = 0;
                    msgTime          = 0;
                    msgTimeFine      = 0;
                    tskTime          = 0;
                    tskTimeFine      = 0;
#endif
                }

#ifdef DEBUG
                unsigned long getMsgTime() {
                    return msgTime + ( msgTimeFine / 1000 );
                }

                unsigned long getTskTime() {
                    return tskTime + ( tskTimeFine / 1000 );
                }
#endif
                entity *      pEnt;
                unsigned long minMicros;
                unsigned int  priority;
                unsigned long lastCall;
                unsigned long lateTime;
#ifdef DEBUG
                unsigned long numberOfCalls;
                unsigned long numberOfMessages;
                unsigned long msgTime;
                unsigned long msgTimeFine;
                unsigned long tskTime;
                unsigned long tskTimeFine;
#endif
            };

            typedef task *                    T_PTASK;
            typedef std::list<T_PTASK>        T_TASKLIST;
            typedef std::list<T_SUBSCRIPTION> T_SUBSCRIPTIONLIST;

            // members
            T_TASKLIST         taskList;
            T_SUBSCRIPTIONLIST subscriptionList;
#ifdef DEBUG
            public:
            unsigned long msgDispatched    = 0;
            unsigned long msgQueueTime     = 0;
            unsigned long msgQueueTimeFine = 0;
            unsigned long taskCalls        = 0;
            unsigned long taskTime         = 0;
            unsigned long taskTimeFine     = 0;
            unsigned long lifeTime         = 0;
            unsigned long lifeTimeFine     = 0;
            unsigned long lifeTimeLast     = micros();

            unsigned long getMsgDispatched() {
                return msgDispatched;
            }

            unsigned long getMsgQueueTime() {
                return msgQueueTime + ( msgQueueTimeFine / 1000 );
            }

            unsigned long getTaskCalls() {
                return taskCalls;
            }

            unsigned long getTaskTime() {
                return taskTime + ( taskTimeFine / 1000 );
            }

            unsigned long getLifeTime() {
                return lifeTime + ( lifeTimeFine / 1000 );
            }

            void dumpInfo( String pre ) {
                DBG( "Task Information" );
                DBG( "----------------" );
                DBG( pre + "Dispatched Messages: " + getMsgDispatched() );
                DBG( pre + "Dispatched Tasks: " + getTaskCalls() );
                DBG( pre + "Message Time: " + getMsgQueueTime() + " ms" );
                DBG( pre + "Task Time: " + getTaskTime() + " ms" );
                DBG( pre + "Tital Time: " + getLifeTime() + " ms" );
                DBG( "" );
                DBG( pre + "Individual Tast Statistics:" );
                for ( auto pTask : taskList ) {
                    DBG( "" );
                    DBG( pre + "  Name: " + pTask->pEnt->entName );
                    DBG( pre + "  Calls: " + pTask->numberOfCalls );
                    DBG( pre + "  Calls Time: " + pTask->getTskTime() + " ms" );
                    DBG( pre + "  Messages: " + pTask->numberOfMessages );
                    DBG( pre + "  Message Time: " + pTask->getMsgTime() + " ms" );
                }
            }
#endif
            // methods
            public:
            scheduler() {
                taskList.clear();
                subscriptionList.clear();
            }

            virtual ~scheduler() {
                for ( auto pTask : taskList ) {
                    delete pTask;
                }
                taskList.clear();
            }

            void loop() {
                // process entity and kernel tasks
                processMsgQueue();

                // XXX: sort tasks according to urgency
                for ( auto pTask : taskList ) {
                    // process message queue
                    processMsgQueue();
                    // process entity and kernel tasks
                    processTask( pTask );
                }
#ifdef DEBUG
                unsigned long lifeTimeNow = micros();
                lifeTimeFine += superdelta( lifeTimeLast, lifeTimeNow );
                lifeTimeLast = lifeTimeNow;
                if ( lifeTimeFine > 1000000000L ) {
                    lifeTime += ( lifeTimeFine / 1000 );
                    lifeTimeFine = 0;
                }
#endif
            }

            // internal methods
            protected:
            void processMsgQueue() {
#ifdef DEBUG
                unsigned long msgStartTime = micros();
#endif
                for ( message *pMsg = message::que.pop(); pMsg != nullptr;
                      pMsg          = message::que.pop() ) {
#ifdef DEBUG
                    ++msgDispatched;
#endif
                    switch ( pMsg->type ) {
                    case message::MSG_DIRECT:
                        directMsg( pMsg );
                        break;
                    case message::MSG_PUBLISH:
                        publishMsg( pMsg );
                        break;
                    case message::MSG_SUBSCRIBE:
                        subscribeMsg( pMsg );
                        break;
                    default:
                        DBG( "Unexpected message type: " + String( pMsg->type ) );
                        break;
                    }
                    delete pMsg;
                }
#ifdef DEBUG
                msgQueueTimeFine += superdelta( msgStartTime, micros() );
                if ( msgQueueTimeFine > 1000000000L ) {
                    msgQueueTime += ( msgQueueTimeFine / 1000 );
                    msgQueueTimeFine = 0;
                }
#endif
            }

            void processTask( T_PTASK pTask ) {
                unsigned long ticker = micros();
                unsigned long tDelta = superdelta( pTask->lastCall, ticker );
                if ( ( pTask->minMicros > 0 ) && ( tDelta >= pTask->minMicros ) ) {
                    pTask->pEnt->onLoop( ticker );
                    pTask->lastCall = ticker;
                    pTask->lateTime += tDelta - pTask->minMicros;
#ifdef DEBUG
                    unsigned long budget = superdelta( ticker, micros() );
                    // task
                    ++pTask->numberOfCalls;
                    pTask->tskTimeFine += budget;
                    if ( pTask->tskTimeFine > 1000000000L ) {
                        pTask->tskTime += ( pTask->tskTimeFine / 1000 );
                        pTask->tskTimeFine = 0;
                    }
                    // totals
                    ++taskCalls;
                    taskTimeFine += budget;
                    if ( taskTimeFine > 1000000000L ) {
                        taskTime += ( taskTimeFine / 1000 );
                        taskTimeFine = 0;
                    }
#endif
                }
            }

            void directMsg( message *pMsg ) {
                if ( String( pMsg->topic ) == "register" ) {
                    if ( pMsg->pBufLen != sizeof( msgregister ) ) {
                        DBG( "Direct message: invalid message buffer size!" +
                             String( pMsg->topic ) );
                    } else {
                        msgregister *pReg = (msgregister *)pMsg->pBuf;
                        registerEntity( pReg->pEnt, pReg->minMicroSecs, pReg->priority );
                        DBG( "Registered entity: " + String( pReg->pEnt->entName ) );
                    }
                } else {
                    DBG( "Direct message: not implemented: " + String( pMsg->topic ) );
                }
            }

            void publishMsg( message *pMsg ) {
                for ( auto sub : subscriptionList ) {
                    String topic( pMsg->topic );
                    if ( msgmatches( sub.topic, topic ) ) {
                        for ( auto pTask : taskList ) {
                            if ( pTask->pEnt->entName == sub.subscriber ) {
#ifdef DEBUG
                                unsigned long ticker = micros();
                                pTask->pEnt->onReceiveMessage( topic, pMsg->pBuf, pMsg->pBufLen );
                                ++pTask->numberOfMessages;
                                pTask->msgTimeFine += superdelta( ticker, micros() );
                                if ( pTask->msgTimeFine > 1000000000L ) {
                                    pTask->msgTime += ( pTask->msgTimeFine / 1000 );
                                    pTask->msgTimeFine = 0;
                                }
#else
                                pTask->pEnt->onReceiveMessage( topic, pMsg->pBuf, pMsg->pBufLen );
#endif
                            }
                        }
                    }
                }
            }

            void subscribeMsg( message *pMsg ) {
                T_SUBSCRIPTION subs;
                subs.subscriber = pMsg->originator;
                subs.topic      = pMsg->topic;
                subscriptionList.emplace_back( subs );
            }

            bool registerEntity( entity *pEnt, unsigned long minMicroSecs = 100000L,
                                 unsigned int priority = PRIORITY_NORMAL ) {
                task *pTask = new task( pEnt, minMicroSecs, priority );
                if ( pTask == nullptr ) {
                    return false;
                }
                taskList.push_back( pTask );
                pEnt->onSetup();
                return true;
            }

            bool msgmatches( String tSearch, String topic ) {
                // XXX Implement serious pattern matching
                if ( tSearch == "*" ) {
                    return true;
                } else if ( tSearch == topic ) {
                    return true;
                }
                return false;
            }
        };
    } // namespace core
} // namespace meisterwerk
#endif
