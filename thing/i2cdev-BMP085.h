// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <Adafruit_BMP085.h>
#include <ESP8266WiFi.h>

// external libraries
#include <ArduinoJson.h>

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

            i2cdev_BMP085( String name )
                : meisterwerk::base::i2cdev( name, "BMP085" ), tempProcessor( 5, 900, 0.1 ),
                  pressProcessor( 10, 900, 0.05 ) {
                // send temperature updates, if temperature changes for 0.1C over an average of 5
                // measurements, but at least every 15min (900sec)
                // send pressure update, if pressure changes for 0.05mbar over an average of 10
                // measurements, but at least every 15min
            }
            ~i2cdev_BMP085() {
                if ( pollSensor ) {
                    pollSensor = false;
                    delete pbmp;
                }
            }

            bool registerEntity() {
                // 5sec sensor checks
                return meisterwerk::core::entity::registerEntity( 5000000 );
            }

            virtual void onInstantiate( String i2ctype, uint8_t address ) override {
                DBG( "Instantiating BMP085 device at address 0x" +
                     meisterwerk::util::hexByte( address ) );
                pbmp = new Adafruit_BMP085();
                if ( !pbmp->begin() ) {
                    DBG( "BMP085 initialization failure." );
                } else {
                    pollSensor = true;
                    subscribe( entName + "/config" );
                }
            }

            virtual void onLoop( unsigned long ticker ) override {
                if ( pollSensor ) {
                    float temperature = pbmp->readTemperature();
                    if ( tempProcessor.filter( &temperature ) ) {
                        json = "{\"temperature\":" + String( temperature ) + "}";
                        DBG( "jsonstate i2c bmp085:" + json );
                        publish( entName + "/temperature", json );
                    }
                    float pressure = pbmp->readPressure() / 100.0;
                    if ( pressProcessor.filter( &pressure ) ) {
                        json = "{\"pressure\":" + String( pressure ) + "}";
                        DBG( "jsonstate i2c bmp085:" + json );
                        publish( entName + "/pressure", json );
                    }
                }
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                meisterwerk::base::i2cdev::onReceive( origin, topic, msg );
                if ( topic == entName + "/config" ) {
                    config( msg );
                }
            }

            void config( String msg ) {
                // XXX: do things
            }
        };
    } // namespace base
} // namespace meisterwerk
