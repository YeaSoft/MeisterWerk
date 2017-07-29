// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <Adafruit_BMP085.h>

// dependencies
#include "../base/i2cdev.h"
#include "../core/entity.h"
#include "../util/hextools.h"
#include "../util/sensorprocessor.h"

namespace meisterwerk {
    namespace thing {
        class i2cdev_BMP085 : public meisterwerk::base::i2cdev {
            public:
            Adafruit_BMP085 *                  pbmp;
            bool                               pollSensor = false;
            meisterwerk::util::sensorprocessor tempProcessor, pressProcessor;
            String                             json;
            bool                               tempvalid;
            double                             templast;
            String                             temptime;
            bool                               pressvalid;
            double                             presslast;
            String                             presstime;
            bool                               bOptionWaitForValidTime = true;
            bool                               bTimeValid              = false;

            i2cdev_BMP085( String name, uint8_t address )
                : meisterwerk::base::i2cdev( name, "BMP085", address ), tempProcessor( 5, 900, 0.1 ),
                  pressProcessor( 10, 900, 0.05 ) {
                // send temperature updates, if temperature changes for 0.1C over an average of 5
                // measurements, but at least every 15min (900sec)
                // send pressure update, if pressure changes for 0.05mbar over an average of 10
                // measurements, but at least every 15min
                tempvalid  = false;
                pressvalid = false;
            }
            ~i2cdev_BMP085() {
                if ( pollSensor ) {
                    pollSensor = false;
                    delete pbmp;
                }
            }

            virtual void setup() override {
            }

            virtual void onInstantiate( String i2ctype, uint8_t address ) override {
                // String sa = meisterwerk::util::hexByte( address );
                DBG( "Instantiating BMP085 device at address 0x" + meisterwerk::util::hexByte( address ) );
                pbmp = new Adafruit_BMP085();
                if ( !pbmp->begin() ) {
                    DBG( "BMP085 initialization failure." );
                } else {
                    pollSensor = true;
                    subscribe( entName + "/temperature/get" );
                    subscribe( entName + "/pressure/get" );
                    subscribe( "mastertime/time/set" );
                }
            }

            void publishTemp() {
                if ( tempvalid ) {
                    json = "{\"time\":\"" + temptime + "\",\"temperature\":" + String( templast ) + "}";
                    // DBG( "jsonstate i2c bmp085:" + json );
                    publish( entName + "/temperature", json );
                } else {
                    DBG( "No valid temp measurement for pub" );
                }
            }
            void publishPressure() {
                if ( pressvalid ) {
                    json = "{\"time\":\"" + presstime + "\",\"pressure\":" + String( presslast ) + "}";
                    // DBG( "jsonstate i2c bmp085:" + json );
                    publish( entName + "/pressure", json );
                } else {
                    DBG( "No valid pressure measurement for pub" );
                }
            }
            virtual void loop() override {
                if ( pollSensor ) {
                    if ( timeStatus() != timeNotSet )
                        bTimeValid = true;
                    if ( bOptionWaitForValidTime && !bTimeValid )
                        return;
                    double temperature = pbmp->readTemperature();
                    if ( tempProcessor.filter( &temperature ) ) {
                        templast  = temperature;
                        tempvalid = true;
                        temptime  = util::msgtime::time_t2ISO( now() );
                        publishTemp();
                    }
                    double pressure = pbmp->readPressure() / 100.0;
                    if ( pressProcessor.filter( &pressure ) ) {
                        presslast  = pressure;
                        pressvalid = true;
                        presstime  = util::msgtime::time_t2ISO( now() );
                        publishPressure();
                    }
                }
            }

            virtual void receive( const char *origin, const char *ctopic, const char *msg ) override {
                meisterwerk::base::i2cdev::receive( origin, ctopic, msg );
                String topic( ctopic );
                if ( topic == "mastertime/time/set" )
                    bTimeValid = true;
                if ( topic == entName + "/temperature/get" || topic == "*/temperature/get" ) {
                    publishTemp();
                }
                if ( topic == entName + "/pressure/get" || topic == "*/pressure/get" ) {
                    publishPressure();
                }
            }
        };
    } // namespace thing
} // namespace meisterwerk
