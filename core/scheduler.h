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
#include "array.h"
#include "common.h"
#include "entity.h"
#include "topic.h"

namespace meisterwerk {
    namespace core {

        class baseapp;

        class scheduler {
            friend class baseapp;

            // internal types
            protected:
            class subscription {
                public:
                String subscriber;
                String topicmask;
                subscription() {
                    subscriber = "", topicmask = "";
                }
                subscription( String sub, String top ) : subscriber{sub}, topicmask{top} {
                }
            };

            class task {
                public:
                task() {
                    pEnt      = nullptr;
                    minMicros = 0;
                    priority  = PRIORITY_LOWEST;
                    lastCall  = 0;
                    lateTime  = 0;
                }
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

            // members
            array<task>         taskList;
            array<subscription> subscriptionList;

            meisterwerk::util::metronome yieldRythm = 5; // 5ms

            // methods
            public:
            scheduler( int nTaskListSize = 32, int nSubscriptionListSize = 128, int nRetainPubs = 32 )
                : taskList( nTaskListSize ), subscriptionList( nSubscriptionListSize ) {
                DBG_ONLY( allTime.snap() );
#ifdef ESP8266
                ESP.wdtDisable();
                ESP.wdtEnable( WDTO_8S );
#endif
            }

            virtual ~scheduler() {
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
                for ( unsigned int i = 0; i < taskList.length(); i++ ) {
                    // process message queue
                    processMsgQueue();
                    // process entity and kernel tasks
                    processTask( &taskList[i] );
                    // serve the watchdog
                    checkYield();
                }
#ifdef ESP8266
                ESP.wdtFeed();
#endif
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

            void processTask( task *pTask ) {
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
                for ( unsigned int isub = 0; isub < subscriptionList.length(); isub++ ) {
                    if ( Topic::mqttmatch( pMsg->topic, subscriptionList[isub].topicmask ) ) {
                        for ( unsigned int itask = 0; itask < taskList.length(); itask++ ) {
                            if ( ( taskList[itask].pEnt->entName == subscriptionList[isub].subscriber ) &&
                                 ( String( pMsg->originator ) != subscriptionList[isub].subscriber ) ) {
                                DBG_ONLY( taskList[itask].msgTime.snap() );
                                taskList[itask].pEnt->receive( pMsg->originator, pMsg->topic,
                                                               pMsg->pBuf && pMsg->pBufLen ? (const char *)pMsg->pBuf
                                                                                           : "" );
                                DBG_ONLY( taskList[itask].msgTime.shot() );
                            }
                        }
                    }
                }
            }

            bool subscribeMsg( message *pMsg ) {
                subscription Subs( pMsg->originator, pMsg->topic );
                bool         ret = subscriptionList.add( Subs );
                return ret;
            }

            void unsubscribeMsg( message *pMsg ) {
                for ( unsigned int i = 0; i < subscriptionList.length(); i++ ) {
                    if ( ( subscriptionList[i].topicmask == String( pMsg->topic ) ) &&
                         ( subscriptionList[i].subscriber == String( pMsg->originator ) ) ) {
                        subscriptionList.erase( i );
                        return;
                    }
                }
                DBG( "Entity " + String( pMsg->originator ) + " tried to unsubcribe topic " + String( pMsg->topic ) +
                     " which had not been subscribed!" );
                return;
            }

            bool registerEntity( entity *pEnt, unsigned long minMicroSecs = 100000L, T_PRIO priority = PRIORITY_NORMAL,
                                 bool bCallback = true ) {
                for ( unsigned int i = 0; i < taskList.length(); i++ ) {
                    if ( taskList[i].pEnt->entName == pEnt->entName ) {
                        DBG( "ERROR: cannot register another task with existing entity-name: " + pEnt->entName );
                        return false;
                    }
                }
                task *pTask = new task( pEnt, minMicroSecs, priority );
                if ( pTask == nullptr ) {
                    return false;
                }
                taskList.add( *pTask );

                if ( bCallback ) {
                    pEnt->setup();
                }
                return true;
            }

            bool updateEntity( entity *pEnt, unsigned long minMicroSecs = 100000L, T_PRIO priority = PRIORITY_NORMAL ) {
                bool found = false;
                for ( unsigned int i = 0; i < taskList.length(); i++ ) {
                    if ( taskList[i].pEnt->entName == pEnt->entName ) {
                        taskList[i].minMicros = minMicroSecs;
                        taskList[i].priority  = priority;
                        found                 = true;
                        break;
                    }
                }
                if ( !found ) {
                    DBG( "ERROR: cannot updateEntity for not existing entity-name: " + pEnt->entName );
                    return false;
                }
                return true;
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
                for ( unsigned int i = 0; i < subscriptionList.length(); i++ ) {
                    DBG( pre + "subscriber='" + subscriptionList[i].subscriber + "' topic='" +
                         subscriptionList[i].topicmask + "'" );
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
                for ( unsigned int i = 0; i < taskList.length(); i++ ) {
                    DBG( "" );
                    DBG( pre + F( "  Name: " ) + taskList[i].pEnt->entName );
                    DBG( pre + F( "  Calls: " ) + taskList[i].tskTime.getcount() );
                    DBG( pre + F( "  Calls Time: " ) + taskList[i].tskTime.getms() + ms + " (" +
                         taskList[i].tskTime.getPercent( allTime.getms() ) + "%)" );
                    DBG( pre + F( "  Calls Max Time: " ) + taskList[i].tskTime.getmaxus() + us );
                    DBG( pre + F( "  Messages: " ) + taskList[i].msgTime.getcount() );
                    DBG( pre + F( "  Message Time: " ) + taskList[i].msgTime.getms() + ms + " (" +
                         taskList[i].msgTime.getPercent( allTime.getms() ) + "%)" );
                    DBG( pre + F( "  Message Max Time: " ) + taskList[i].msgTime.getmaxus() + us );
                }
            }
#endif
        };
    } // namespace core
} // namespace meisterwerk
