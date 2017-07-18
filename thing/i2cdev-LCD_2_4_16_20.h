// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <ESP8266WiFi.h>

// The LCD libraries are some serious mess, totally different implementations with
// different APIs have same name etc. use #576.
//#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal_PCF8574.h>

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
            // LiquidCrystal_I2C *plcd;
            LiquidCrystal_PCF8574 *plcd;
            bool                   pollDisplay = false;
            // meisterwerk::util::sensorprocessor tempProcessor, pressProcessor;
            String  json;
            String  dispSize;
            uint8_t adr;
            uint8_t instAddress;

            i2cdev_LCD_2_4_16_20( String name, uint8_t address,
                                  String dispSize ) // "2x16" or "4x20"
                : meisterwerk::base::i2cdev( name, "LCD_2_4_16_20", address ),
                  instAddress{address}, dispSize{dispSize} {
                DBG( "Constr. OLED" );
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
                if ( address != instAddress ) {
                    return; // not me.
                }
                if ( pollDisplay ) {
                    DBG( "Tried to re-instanciate object: {" + i2ctype + "," + entName +
                         "} at address 0x" + meisterwerk::util::hexByte( address ) +
                         "Display: " + dispSize );
                    return;
                }
                // String sa = meisterwerk::util::hexByte( address );
                DBG( "Instantiating LCD_2_4_16_20 device {" + i2ctype + "," + entName +
                     "} at address 0x" + meisterwerk::util::hexByte( address ) + ", " + dispSize );
                adr = address;
                if ( dispSize == "2x16" ) {
                    // plcd = new LiquidCrystal_I2C( address, 16, 2 ); // 0: default address;
                    plcd = new LiquidCrystal_PCF8574( address ); // 0: default address;
                    plcd->begin( 16, 2 );
                } else if ( dispSize == "4x20" ) {
                    // plcd = new LiquidCrystal_I2C( address, 20, 4 ); // 0: default address;
                    plcd = new LiquidCrystal_PCF8574( address ); // 0: default address;
                    plcd->begin( 20, 4 );
                } else {
                    DBG( "Uknown display size, cannot instantiate LCD entity: ERROR" );
                    return;
                }
                // plcd->init();
                // plcd->backlight();
                plcd->setBacklight( 255 );
                plcd->home();
                plcd->clear();
                plcd->print( entName + ", " + dispSize + ", 0x" +
                             meisterwerk::util::hexByte( address ) );
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
                    // plcd->print( String( l ) );
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
                    // plcd->print( "Hello, " + entName + ", size: " + dispSize + ", 0x" +
                    //             meisterwerk::util::hexByte( adr ) );
                }
            }

            void config( String msg ) {
                // XXX: do things
            }
        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
