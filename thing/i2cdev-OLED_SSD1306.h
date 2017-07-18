// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <ESP8266WiFi.h>

#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../base/i2cdev.h"
#include "../core/entity.h"
#include "../util/hextools.h"

namespace meisterwerk {
    namespace thing {
        class i2cdev_OLED_SSD1306 : public meisterwerk::base::i2cdev {
            public:
            // LiquidCrystal_I2C *plcd;
            Adafruit_SSD1306 *poled;
            bool              pollDisplay = false;
            // meisterwerk::util::sensorprocessor tempProcessor, pressProcessor;
            String  json;
            String  dispSize;
            uint8_t adr;
            uint8_t instAddress;

            i2cdev_OLED_SSD1306( String name, uint8_t address,
                                 String dispSize ) // "128x64"..
                : meisterwerk::base::i2cdev( name, "SSD1306", address ),
                  instAddress{address}, dispSize{dispSize} {
            }
            ~i2cdev_OLED_SSD1306() {
                if ( pollDisplay ) {
                    pollDisplay = false;
                    delete poled;
                }
            }

            bool registerEntity() {
                // 5sec sensor checks
                bool ret = meisterwerk::core::entity::registerEntity( 5000000 );
                return ret;
            }

            virtual void onInstantiate( String i2ctype, uint8_t address ) override {
                if ( address != instAddress ) {
                    DBG( "Ignoring instantiate for " + meisterwerk::util::hexByte( address ) +
                         " I'm " + meisterwerk::util::hexByte( instAddress ) );
                    return; // not me.
                }
                if ( pollDisplay ) {
                    DBG( "Tried to re-instanciate object: {" + i2ctype + "," + entName +
                         "} at address 0x" + meisterwerk::util::hexByte( address ) +
                         "Display: " + dispSize );
                    return;
                }
                // String sa = meisterwerk::util::hexByte( address );
                DBG( "Instantiating OLED_SSD1306 device {" + i2ctype + "," + entName +
                     "} at address 0x" + meisterwerk::util::hexByte( address ) + ", " + dispSize );
                adr = address;
                // #define OLED_RESET 4
                DBG( "init oled." );
                poled = new Adafruit_SSD1306();
                poled->begin( SSD1306_SWITCHCAPVCC,
                              address ); // initialize with the I2C addr 0x3D (for the 128x64)
                                         // poled->clear();
                poled->clearDisplay();
                poled->setTextSize( 1 );
                poled->setTextColor( WHITE );
                poled->setCursor( 0, 0 );
                poled->println( "Hello, world!" );
                poled->println( "Hello, world!" );
                poled->display();
                // poled->clearDisplay();

                DBG( "Instance OLED on." );
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
                    poled->println( "OLED online." );
                    poled->display();
                }
            }

            void config( String msg ) {
                // XXX: do things
            }
        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
