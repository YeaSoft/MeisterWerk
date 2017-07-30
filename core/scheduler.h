// scheduler.h - The internal queue class
//
// This is the declaration of the internal scheduler
// class that is part of the implementation of the
// application method for non blocking communication
// between the components and scheduling

#pragma once

// dependencies
#include "../util/metronome.h"
#include "../util/timebudget.h"
#include "common.h"
#include "entity.h"
#include "topic.h"
#include <list>

namespace meisterwerk {
    namespace core {

        class baseapp;

        class scheduler {
            friend class baseapp;

            // internal types
            protected:
            typedef struct {
                String subscriber;
                String topic;
            } T_SUBSCRIPTION;

            class task {
                public:
                task( entity *pEnt, unsigned long minMicros, T_PRIO priority )
                    : pEnt{pEnt}, minMicros{minMicros}, priority{priority} {
                    lastCall = 0;
                    lateTime = 0;
                }

                entity *      pEnt;
                unsigned long minMicros;
                T_PRIO        priority;
                unsigned long lastCall;
                unsigned long lateTime;

                DBG_ONLY( meisterwerk::util::timebudget msgTime );
                DBG_ONLY( meisterwerk::util::timebudget tskTime );
            };

            typedef task *                    T_PTASK;
            typedef std::list<T_PTASK>        T_TASKLIST;
            typedef std::list<T_SUBSCRIPTION> T_SUBSCRIPTIONLIST;

            // members
            T_TASKLIST                   taskList;
            T_SUBSCRIPTIONLIST           subscriptionList;
            meisterwerk::util::metronome yieldRythm = 5; // 5ms

            // methods
            public:
            scheduler() {
                taskList.clear();
                subscriptionList.clear();
                DBG_ONLY( allTime.snap() );
                ESP.wdtDisable();
                ESP.wdtEnable( WDTO_8S );
            }

            virtual ~scheduler() {
                for ( auto pTask : taskList ) {
                    delete pTask;
                }
                taskList.clear();
            }

            void checkYield() {
                if ( yieldRythm.woof() ) {
                    yield();
                }
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
                    // serve the watchdog
                    checkYield();
                }
                ESP.wdtFeed();
                DBG_ONLY( allTime.shot() );
            }

            // internal methods
            protected:
            void processMsgQueue() {
                DBG_ONLY( msgTime.snap() );
                for ( message *pMsg = message::que.pop(); pMsg != nullptr; pMsg = message::que.pop() ) {
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
                    case message::MSG_UNSUBSCRIBE:
                        unsubscribeMsg( pMsg );
                        break;
                    default:
                        DBG( "Unexpected message type: " + String( pMsg->type ) );
                        break;
                    }
                    delete pMsg;
                    checkYield();
                    DBG_ONLY( msgTime.shot() );
                }
            }

            void processTask( T_PTASK pTask ) {
                unsigned long ticker = micros();
                unsigned long tDelta = meisterwerk::util::timebudget::delta( pTask->lastCall, ticker );
                if ( ( pTask->minMicros > 0 ) && ( tDelta >= pTask->minMicros ) ) {
                    DBG_ONLY( tskTime.snap() );
                    DBG_ONLY( pTask->tskTime.snap() );

                    pTask->pEnt->loop();

                    DBG_ONLY( pTask->tskTime.shot() );
                    DBG_ONLY( tskTime.shot() );

                    pTask->lastCall = ticker;
                    pTask->lateTime += tDelta - pTask->minMicros;
                }
            }

            void directMsg( message *pMsg ) {
                if ( String( pMsg->topic ) == "register" ) {
                    if ( pMsg->pBufLen != sizeof( msgregister ) ) {
                        DBG( "Direct message: invalid reg message buffer size!" + String( pMsg->topic ) );
                    } else {
                        msgregister *pReg = (msgregister *)pMsg->pBuf;
                        registerEntity( pReg->pEnt, pReg->minMicroSecs, pReg->priority );
                        DBG( "Registered entity: " + String( pReg->pEnt->entName ) +
                             ", Slice: " + String( pReg->minMicroSecs ) );
                    }
                } else if ( String( pMsg->topic ) == "update" ) {
                    if ( pMsg->pBufLen != sizeof( msgregister ) ) {
                        DBG( "Direct message: invalid updateEntity message buffer size!" + String( pMsg->topic ) );
                    } else {
                        msgregister *pReg = (msgregister *)pMsg->pBuf;
                        updateEntity( pReg->pEnt, pReg->minMicroSecs, pReg->priority );
                        DBG( "updateEntity: " + String( pReg->pEnt->entName ) );
                    }
                } else {
                    DBG( "Direct message: not implemented: " + String( pMsg->topic ) );
                }
            }

            void publishMsg( message *pMsg ) {
                for ( auto sub : subscriptionList ) {
                    if ( msgmatches( sub.topic, pMsg->topic ) ) {
                        for ( auto pTask : taskList ) {
                            if ( ( pTask->pEnt->entName == sub.subscriber ) &&
                                 ( String( pMsg->originator ) != sub.subscriber ) ) {
                                DBG_ONLY( pTask->msgTime.snap() );
                                pTask->pEnt->receive( pMsg->originator, pMsg->topic,
                                                      pMsg->pBuf && pMsg->pBufLen ? (const char *)pMsg->pBuf : "" );
                                DBG_ONLY( pTask->msgTime.shot() );
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

            void unsubscribeMsg( message *pMsg ) {
                T_SUBSCRIPTIONLIST::iterator iter = subscriptionList.begin();
                while ( iter != subscriptionList.end() ) {
                    T_SUBSCRIPTION sub = ( *iter );
                    // if ( msgmatches( sub.topic, pMsg->topic ) ) {  // Wild-card unsubscribe not
                    // supported.
                    if ( ( sub.topic == String( pMsg->topic ) ) && ( sub.subscriber == String( pMsg->originator ) ) ) {
                        subscriptionList.erase( iter );
                        return;
                    }
                    ++iter;
                }
                DBG( "Entity " + String( pMsg->originator ) + " tried to unscribe topic " + String( pMsg->topic ) +
                     " which had not been subscribed!" );
                return;
            }

            bool registerEntity( entity *pEnt, unsigned long minMicroSecs = 100000L, T_PRIO priority = PRIORITY_NORMAL,
                                 bool bCallback = true ) {
                for ( auto pTask : taskList ) {
                    if ( pTask->pEnt->entName == pEnt->entName ) {
                        DBG( "ERROR: cannot register another task with existing entity-name: " + pEnt->entName );
                        return false;
                    }
                }
                task *pTask = new task( pEnt, minMicroSecs, priority );
                if ( pTask == nullptr ) {
                    return false;
                }
                taskList.push_back( pTask );

                if ( bCallback ) {
                    pEnt->setup();
                }
                return true;
            }

            bool updateEntity( entity *pEnt, unsigned long minMicroSecs = 100000L, T_PRIO priority = PRIORITY_NORMAL ) {
                bool    found = false;
                T_PTASK pTask;
                for ( auto pt : taskList ) {
                    if ( pt->pEnt->entName == pEnt->entName ) {
                        pTask = pt;
                        found = true;
                    }
                }
                if ( !found ) {
                    DBG( "ERROR: cannot updateEntity for not existing entity-name: " + pEnt->entName );
                    return false;
                }
                pTask->minMicros = minMicroSecs;
                pTask->priority  = priority;
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
                    l = l1;
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

#ifdef _MW_DEBUG
            public:
            meisterwerk::util::timebudget msgTime;
            meisterwerk::util::timebudget tskTime;
            meisterwerk::util::timebudget allTime;

            void dumpInfo( String pre ) {
                const __FlashStringHelper *ms = F( " ms" );
                const __FlashStringHelper *us = F( " us" );
                DBG( "" );
                DBG( F( "Subscriptions" ) );
                DBG( F( "=============" ) );
                for ( auto sub : subscriptionList ) {
                    DBG( pre + "subscriber='" + sub.subscriber + "' topic='" + sub.topic + "'" );
                }

                DBG( "" );
                DBG( F( "Task Information" ) );
                DBG( F( "================" ) );
                DBG( pre + F( "Dispatched Messages: " ) + msgTime.getcount() );
                DBG( pre + F( "Dispatched Tasks: " ) + tskTime.getcount() );
                DBG( pre + F( "Message Time: " ) + msgTime.getms() + ms );
                DBG( pre + F( "Task Time: " ) + tskTime.getms() + ms + " (" + tskTime.getPercent( allTime.getms() ) +
                     "%)" );
                DBG( pre + F( "Total Time: " ) + allTime.getms() + ms );
                DBG( "" );
                DBG( pre + F( "Individual Task Statistics:" ) );
                DBG( pre + F( "---------------------------" ) );
                for ( auto pTask : taskList ) {
                    DBG( "" );
                    DBG( pre + F( "  Name: " ) + pTask->pEnt->entName );
                    DBG( pre + F( "  Calls: " ) + pTask->tskTime.getcount() );
                    DBG( pre + F( "  Calls Time: " ) + pTask->tskTime.getms() + ms + " (" +
                         pTask->tskTime.getPercent( allTime.getms() ) + "%)" );
                    DBG( pre + F( "  Calls Max Time: " ) + pTask->tskTime.getmaxus() + us );
                    DBG( pre + F( "  Messages: " ) + pTask->msgTime.getcount() );
                    DBG( pre + F( "  Message Time: " ) + pTask->msgTime.getms() + ms + " (" +
                         pTask->msgTime.getPercent( allTime.getms() ) + "%)" );
                    DBG( pre + F( "  Message Max Time: " ) + pTask->msgTime.getmaxus() + us );
                }
            }
#endif
        };
    } // namespace core
} // namespace meisterwerk
