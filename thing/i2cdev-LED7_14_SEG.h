// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <Adafruit_LEDBackpack.h>
#include <ESP8266WiFi.h>

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../base/i2cdev.h"
#include "../core/entity.h"
#include "../util/hextools.h"

namespace meisterwerk {
    namespace thing {
        class i2cdev_LED7_14_SEG : public meisterwerk::base::i2cdev {
            public:
            Adafruit_AlphaNum4 *pled;
            bool                pollDisplay = false;
            uint8_t             address;
            // meisterwerk::util::sensorprocessor tempProcessor, pressProcessor;
            String json;

            i2cdev_LED7_14_SEG( String name, uint8_t address )
                : meisterwerk::base::i2cdev( name, "LED7_14_SEG", address ), address{address} {
            }
            ~i2cdev_LED7_14_SEG() {
                if ( pollDisplay ) {
                    pollDisplay = false;
                    delete pled;
                }
            }

            bool registerEntity() {
                // 5sec sensor checks
                bool ret = meisterwerk::core::entity::registerEntity( 5000000 );
                return ret;
            }

            virtual void onInstantiate( String i2ctype, uint8_t address ) override {
                // String sa = meisterwerk::util::hexByte( address );
                DBG( "Instantiating LED7_14_SEG device at address 0x" +
                     meisterwerk::util::hexByte( address ) );
                pled = new Adafruit_AlphaNum4();
                pled->begin( address );
                DBG( "Instance 7/14-LED on." );
                pollDisplay = true;
                subscribe( entName + "/config" );
                subscribe( "textdisplay/enum" );
                subscribe( entName + "/display" );
                publish( entName + "/textdisplay" );
            }

            virtual void onLoop( unsigned long ticker ) override {
                if ( pollDisplay ) {
                    // Do things.
                }
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                meisterwerk::base::i2cdev::onReceive( origin, topic, msg );
                DBG( "Display: " + topic );
                if ( topic == entName + "/config" ) {
                    config( msg );
                } else if ( topic == "textdisplay/enum" ) {
                    publish( entName + "/textdisplay" );
                } else if ( topic == entName + "/display" ) {
                    pled->clear();
                    pled->writeDigitAscii( 0, 'a' );
                    pled->writeDigitAscii( 1, 'b' );
                    pled->writeDigitAscii( 2, 'c' );
                    pled->writeDigitAscii( 3, 'd' );
                    pled->writeDisplay();
                }
            }

            void config( String msg ) {
                // XXX: do things
            }
        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
