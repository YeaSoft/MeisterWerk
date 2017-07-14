// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../base/i2cdev.h"
#include "../core/entity.h"
#include "../util/hextools.h"

namespace meisterwerk {
    namespace thing {
        class i2cdev_LCD_2_4_16_20 : public meisterwerk::base::i2cdev {
            public:
            LiquidCrystal_I2C *plcd;
            bool               pollDisplay = false;
            // meisterwerk::util::sensorprocessor tempProcessor, pressProcessor;
            String json;

            i2cdev_LCD_2_4_16_20( String name )
                : meisterwerk::base::i2cdev( name, "LCD_2_4_16_20" ) {
            }
            ~i2cdev_LCD_2_4_16_20() {
                if ( pollDisplay ) {
                    pollDisplay = false;
                    delete plcd;
                }
            }

            bool registerEntity() {
                // 5sec sensor checks
                bool ret = meisterwerk::core::entity::registerEntity( 5000000 );
                return ret;
            }

            virtual void onInstantiate( String i2ctype, uint8_t address ) override {
                // String sa = meisterwerk::util::hexByte( address );
                DBG( "Instantiating LCD_2_4_16_20 device at address 0x" +
                     meisterwerk::util::hexByte( address ) );
                plcd = new LiquidCrystal_I2C( 0x27, 16, 2 ); // 0: default address;
                plcd->begin( 16, 2 );
                // plcd->setBacklight( HIGH );
                // plcd->setCursor( 0, 0 );
                plcd->init();
                plcd->backlight();
                plcd->print( "test" );
                DBG( "Instance LCD on." );
                pollDisplay = true;
                subscribe( entName + "/config" );
                subscribe( "textdisplay/enum" );
                subscribe( entName + "/display" );
                publish( entName + "/textdisplay" );
            }

            int          l = 0;
            virtual void onLoop( unsigned long ticker ) override {
                if ( pollDisplay ) {
                    plcd->print( String( l ) );
                    l++;
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
                    DBG( "Now there should be something!" );
                    plcd->print( "Hello, world!" );
                }
            }

            void config( String msg ) {
                // XXX: do things
            }
        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
