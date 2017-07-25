// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <ESP8266WiFi.h>

#include <RTClib.h>

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../base/i2cdev.h"
#include "../core/entity.h"
#include "../util/hextools.h"
#include "../util/sensorprocessor.h"

namespace meisterwerk {
    namespace thing {
        class i2cdev_RTC_DS3231 : public meisterwerk::base::i2cdev {
            public:
            RTC_DS3231 *                       prtc3;
            RTC_DS1307 *                       prtc1;
            bool                               pollSensor = false;
            meisterwerk::util::sensorprocessor tempProcessor;
            String                             json;
            bool                               tempvalid;
            double                             templast;
            String                             temptime;
            util::metronome                    rtcTicker;
            String                             model;

            bool bRtcTimeValid = false;

            bool bOptionWaitForValidTime = true;
            bool bTimeValid              = false;
            bool bGetTime                = false;

            i2cdev_RTC_DS3231( String name, String model, uint8_t address )
                : meisterwerk::base::i2cdev( name, "DS1307_3231", address ),
                  tempProcessor( 5, 900, 0.1 ), rtcTicker( 30000 ), model{model} {
                // send temperature updates, if temperature changes for 0.1C over an average of 5
                // measurements, but at least every 15min (900sec)
                tempvalid = false;
            }
            ~i2cdev_RTC_DS3231() {
                if ( pollSensor ) {
                    pollSensor = false;
                    if ( model == "DS3231" ) {
                        delete prtc3;
                    } else if ( model == "DS1307" ) {
                        delete prtc1;
                    }
                }
            }

            bool registerEntity() {
                // default sample rate: 5s
                return meisterwerk::core::entity::registerEntity( 1000000L );
            }

            virtual void onInstantiate( String i2ctype, uint8_t address ) override {
                // String sa = meisterwerk::util::hexByte( address );
                DBG( "Instantiating RTC_DS3231 device at address 0x" +
                     meisterwerk::util::hexByte( address ) );
                if ( model == "DS3231" ) {
                    prtc3 = new RTC_DS3231();
                    if ( prtc3->begin() ) {
                        if ( prtc3->lostPower() ) {
                            DBG( "RTC lost power, RTC time invalid." );
                        } else {
                            bRtcTimeValid = true;
                            bGetTime      = true;
                            DBG( "RTC-time: " + readTime() );
                        }
                        pollSensor = true;
                    }
                } else if ( model == "DS1307" ) {
                    prtc1 = new RTC_DS1307();
                    if ( prtc1->begin() ) {
                        if ( !prtc1->isrunning() ) {
                            DBG( "RTC lost power, RTC time invalid." );
                        } else {
                            bRtcTimeValid = true;
                            bGetTime      = true;
                            DBG( "RTC-time: " + readTime() );
                        }
                        pollSensor = true;
                    }
                } else {
                    DBG( "RTC DS3231/1307 hardware not found!" );
                    return;
                }
                // subscribe( entName + "/temperature/get" ); // not yet implemented.
                subscribe( entName + "/time/get" );
                subscribe( "mastertime/time/set" );
            }

            void publishTemp() {
                if ( tempvalid ) {
                    json = "{\"time\":\"" + temptime + "\",\"temperature\":" + String( templast ) +
                           "}";
                    publish( entName + "/temperature", json );
                } else {
                    DBG( "No valid temp measurement for pub" );
                }
            }

            void publishTime( String isoTime ) {
                if ( bRtcTimeValid ) {
                    if ( model == "DS3231" ) {
                        json = "{\"time\":\"" + isoTime +
                               "\",\"timesource\":\"HP-RTC\",\"timeprecision\":10000}";
                        publish( entName + "/time", json );
                    } else if ( model == "DS1307" ) {
                        json = "{\"time\":\"" + isoTime +
                               "\",\"timesource\":\"RTC\",\"timeprecision\":10000}";
                        publish( entName + "/time", json );
                    } else {
                        DBG( "Can't publish unknown model: " + model );
                    }
                } else {
                    DBG( "No valid time measurement for pub" );
                }
            }

            virtual void onLoop( unsigned long ticker ) override {
                if ( pollSensor ) {
                    if ( bRtcTimeValid ) {
                        if ( bGetTime || rtcTicker.beat() > 0 ) {
                            bGetTime = false;
                            publishTime( readTime() );
                        }
                    }

                    /*
                    if ( bTimeValid ) {
                        double temperature=prtc->getTemperature(), 2);
                        if ( tempProcessor.filter( &temperature ) ) {
                            templast  = temperature;
                            tempvalid = true;
                            temptime  = util::msgtime::time_t2ISO( now() );
                            publishTemp();
                        }
                    }
                    */
                }
            }

            void setTime( String isoTime ) {
                TimeElements tt;
                DateTime     nowUtc;
                time_t       t = util::msgtime::ISO2time_t( isoTime );
                breakTime( t, tt );

                // following line sets the RTC to the date & time this sketch was compiled
                // January 21, 2014 at 3am you would call:
                // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
                if ( model == "DS3231" ) {
                    prtc3->adjust( DateTime( tt.Year + 1970, tt.Month, tt.Day, tt.Hour, tt.Minute,
                                             tt.Second ) );
                    nowUtc = prtc3->now();
                } else if ( model == "DS1307" ) {
                    prtc1->adjust( DateTime( tt.Year + 1970, tt.Month, tt.Day, tt.Hour, tt.Minute,
                                             tt.Second ) );
                    nowUtc = prtc1->now();
                } else {
                    DBG( "Cannot set time for model: " + model );
                }
                DBG( "Setting RTC clock to: " + isoTime );
                time_t utct = nowUtc.unixtime();
                String iso2 = util::msgtime::time_t2ISO( utct );
                DBG( "Verification: RTC clock is: " + iso2 );
                bRtcTimeValid = true;
            }

            String readTime() {
                DateTime nowUtc;
                if ( model == "DS3231" ) {
                    nowUtc = prtc3->now();
                } else if ( model == "DS1307" ) {
                    nowUtc = prtc1->now();
                } else {
                    DBG( "Cannot get time for model: " + model );
                }
                time_t utct    = nowUtc.unixtime();
                String isoTime = util::msgtime::time_t2ISO( utct );
                return isoTime;
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                meisterwerk::base::i2cdev::onReceive( origin, topic, msg );
                if ( topic == "mastertime/time/set" ) {
                    bTimeValid = true;
                    DynamicJsonBuffer jsonBuffer( 200 );
                    JsonObject &      root = jsonBuffer.parseObject( msg );
                    if ( !root.success() ) {
                        DBG( "RTC: Invalid JSON received: " + msg );
                        return;
                    }
                    String isoTime = root["time"];
                    DBG( "RTC received time: " + isoTime );
                    setTime( isoTime );
                }
                if ( topic == entName + "/temperature/get" || topic == "*/temperature/get" ) {
                    publishTemp();
                }
                if ( topic == entName + "/time/get" || topic == "*/time/get" ) {
                    if ( bRtcTimeValid ) {
                        String isoTime = readTime();
                        publishTime( isoTime );
                    } else {
                        DBG( "RTC: Time requested, but no valid time available." );
                    }
                }
            }
        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
