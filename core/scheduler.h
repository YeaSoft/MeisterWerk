// scheduler.h - The internal queue class
//
// This is the declaration of the internal scheduler
// class that is part of the implementation of the
// application method for non blocking communication
// between the components and scheduling

#pragma once

// dependencies
#include "entity.h"
#include <list>

#ifdef DEBUG
#include "../util/timebudget.h"
#endif

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
                }

                entity *      pEnt;
                unsigned long minMicros;
                unsigned int  priority;
                unsigned long lastCall;
                unsigned long lateTime;
#ifdef DEBUG
                meisterwerk::util::timebudget msgTime;
                meisterwerk::util::timebudget tskTime;
#endif
            };

            typedef task *                    T_PTASK;
            typedef std::list<T_PTASK>        T_TASKLIST;
            typedef std::list<T_SUBSCRIPTION> T_SUBSCRIPTIONLIST;

            // members
            T_TASKLIST         taskList;
            T_SUBSCRIPTIONLIST subscriptionList;

            // methods
            public:
            scheduler() {
                taskList.clear();
                subscriptionList.clear();
#ifdef DEBUG
                allTime.snap();
#endif
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
                allTime.shot();
#endif
            }

            // internal methods
            protected:
            void processMsgQueue() {
#ifdef DEBUG
                msgTime.snap();
#endif
                for ( message *pMsg = message::que.pop(); pMsg != nullptr;
                      pMsg          = message::que.pop() ) {
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
#ifdef DEBUG
                    msgTime.shot();
#endif
                }
            }

            void processTask( T_PTASK pTask ) {
                unsigned long ticker = micros();
                unsigned long tDelta =
                    meisterwerk::util::timebudget::delta( pTask->lastCall, ticker );
                if ( ( pTask->minMicros > 0 ) && ( tDelta >= pTask->minMicros ) ) {
#ifdef DEBUG
                    tskTime.snap();
                    pTask->tskTime.snap();
#endif
                    pTask->pEnt->onLoop( ticker );
#ifdef DEBUG
                    pTask->tskTime.shot();
                    tskTime.shot();
#endif
                    pTask->lastCall = ticker;
                    pTask->lateTime += tDelta - pTask->minMicros;
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
                    if ( msgmatches( sub.topic, pMsg->topic ) ) {
                        for ( auto pTask : taskList ) {
                            if ( (pTask->pEnt->entName == sub.subscriber) && (String(pMsg->originator) != sub.subscriber) ) {
#ifdef DEBUG
                                pTask->msgTime.snap();
                                pTask->pEnt->onReceive( pMsg->originator, pMsg->topic,
                                                        (char *)pMsg->pBuf );
                                pTask->msgTime.shot();
#else
                                pTask->pEnt->onReceive( pMsg->originator, pMsg->topic,
                                                        (char *)pMsg->pBuf );
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
                pEnt->onRegister();
                return true;
            }

            public:
            static bool msgmatches( String s1, String s2 ) {
                // compares topic-paths <subtopic>/<subtopic/...
                // the compare is symmetric, s1==s2 <=> s2==s1.
                // subtopic can be <chars> or <chars>+'*', '*' must be last char of a subtopic.
                // '*' acts within only the current subtopic. Exception: a '*' as last character of
                // a topic-path
                //    matches all deeper subptopics:  a*==a/b/c/d/e, but a*/c1!=a1/b1/c1
                // Samples:   abc/def/ghi == */de*/*, abc/def!=abc, ab*==abc, a*==a/b/c/d
                //    a/b*==a/b/c/d/e, a/b*/d!=a/b/c/d
                if ( s1 == s2 )
                    return true;
                int l1 = s1.length();
                int l2 = s2.length();
                int l;
                if ( l1 < l2 )
                    l = l2;
                else
                    l  = l1;
                int p1 = 0, p2 = 0;
                for ( int i = 0; i < l; l++ ) {
                    if ( ( p1 > l1 ) || ( p2 > l2 ) )
                        return false;
                    if ( s1[p1] == s2[p2] ) {
                        ++p1;
                        ++p2;
                        if ( ( p1 == l1 ) && ( p2 == l2 ) )
                            return true;
                        continue;
                    }
                    if ( ( s1[p1] != '*' ) && ( s2[p2] != '*' ) )
                        return false;
                    if ( s1[p1] == '*' ) {
                        ++p1;
                        if ( p1 == l1 )
                            return true;
                        if ( s1[p1] != '/' )
                            return false;
                        else
                            ++p1;
                        while ( p2 < l2 && s2[p2] != '/' )
                            ++p2;
                        if ( p2 == l2 ) {
                            if ( p1 == l1 )
                                return true;
                            else
                                return false;
                        }
                        if ( s2[p2] != '/' )
                            return false;
                        else
                            ++p2;
                        continue;
                    }
                    if ( s2[p2] == '*' ) {
                        ++p2;
                        if ( p2 == l2 )
                            return true;
                        if ( s2[p2] != '/' )
                            return false;
                        else
                            ++p2;
                        while ( p1 < l1 && s1[p1] != '/' )
                            ++p1;
                        if ( p1 == l1 ) {
                            if ( p2 == l2 )
                                return true;
                            else
                                return false;
                        }
                        if ( s1[p1] != '/' )
                            return false;
                        else
                            ++p1;
                        continue;
                    }
                    return false;
                }
                return false;
            }

#ifdef DEBUG
            public:
            meisterwerk::util::timebudget msgTime;
            meisterwerk::util::timebudget tskTime;
            meisterwerk::util::timebudget allTime;

            void dumpInfo( String pre ) {
                const __FlashStringHelper *ms = F( " ms" );
                const __FlashStringHelper *us = F( " us" );
                DBG( "" );
                DBG( F( "Task Information" ) );
                DBG( F( "================" ) );
                DBG( pre + F( "Dispatched Messages: " ) + msgTime.getcount() );
                DBG( pre + F( "Dispatched Tasks: " ) + tskTime.getcount() );
                DBG( pre + F( "Message Time: " ) + msgTime.getms() + ms );
                DBG( pre + F( "Task Time: " ) + tskTime.getms() + ms );
                DBG( pre + F( "Total Time: " ) + allTime.getms() + ms );
                DBG( "" );
                DBG( pre + F( "Individual Task Statistics:" ) );
                DBG( pre + F( "---------------------------" ) );
                for ( auto pTask : taskList ) {
                    DBG( "" );
                    DBG( pre + F( "  Name: " ) + pTask->pEnt->entName );
                    DBG( pre + F( "  Calls: " ) + pTask->tskTime.getcount() );
                    DBG( pre + F( "  Calls Time: " ) + pTask->tskTime.getms() + ms );
                    DBG( pre + F( "  Calls Max Time: " ) + pTask->tskTime.getmaxus() + us );
                    DBG( pre + F( "  Messages: " ) + pTask->msgTime.getcount() );
                    DBG( pre + F( "  Message Time: " ) + pTask->msgTime.getms() + ms );
                    DBG( pre + F( "  Message Max Time: " ) + pTask->msgTime.getmaxus() + us );
                }
            }
#endif
        };
    } // namespace core
} // namespace meisterwerk
