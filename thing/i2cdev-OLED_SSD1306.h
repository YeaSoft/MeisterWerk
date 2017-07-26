// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <ESP8266WiFi.h>

//#define SSD1306_128_64
//#define SSD1306_128_32
//#define SSD1306_96_16

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
            uint8_t adr;
            uint8_t instAddress;
            int     displayX, displayY;
            bool    bRedraw = false;

            i2cdev_OLED_SSD1306( String name, uint8_t address, int displayY, int displayX )
                : meisterwerk::base::i2cdev( name, "SSD1306", address ),
                  instAddress{address}, displayY{displayY}, displayX{displayX} {
            }
            ~i2cdev_OLED_SSD1306() {
                if ( pollDisplay ) {
                    pollDisplay = false;
                    delete poled;
                }
            }

            bool registerEntity() {
                // 5sec sensor checks
                bool ret = meisterwerk::base::i2cdev::registerEntity( 100000 );
                return ret;
            }

            virtual void onInstantiate( String i2ctype, uint8_t address ) override {
                if ( address != instAddress ) {
                    DBG( "Ignoring instantiate for " + meisterwerk::util::hexByte( address ) + " I'm " +
                         meisterwerk::util::hexByte( instAddress ) );
                    return; // not me.
                }
                if ( pollDisplay ) {
                    DBG( "Tried to re-instanciate object: {" + i2ctype + "," + entName + "} at address 0x" +
                         meisterwerk::util::hexByte( address ) + "Display: " + String( displayY ) + "x" +
                         String( displayX ) );
                    return;
                }
                // String sa = meisterwerk::util::hexByte( address );
                DBG( "Instantiating OLED_SSD1306 device {" + i2ctype + "," + entName + "} at address 0x" +
                     meisterwerk::util::hexByte( address ) + ", " + String( displayY ) + "x" + String( displayX ) );
                adr = address;
                // #define OLED_RESET 4
                poled = new Adafruit_SSD1306();
                poled->begin( SSD1306_SWITCHCAPVCC, address ); // initialize with the I2C addr 0x3D (for the 128x64)
                                                               // poled->clear();
                poled->clearDisplay();
                poled->setTextSize( 1 );
                poled->setTextColor( WHITE );
                poled->setCursor( 0, 0 );
                poled->display();

                pollDisplay = true;
                subscribe( entName + "/display/set" );
                subscribe( entName + "/display/get" );
                publish( entName + "/display" );
            }

            virtual void onLoop( unsigned long ticker ) override {
                if ( bRedraw )
                    poled->display();
            }

            virtual void onReceive( const char *origin, const char *ctopic, const char *msg ) override {
                meisterwerk::base::i2cdev::onReceive( origin, ctopic, msg );
                String topic( ctopic );
                if ( topic == "*/display/get" || topic == entName + "/display/get" ) {
                    publish( entName + "/display",
                             "{\"type\":\"pixeldisplay\",\"x\":" + String( displayX ) + ",\"y\":" + String( displayY ) +
                                 "}" );
                }
                if ( topic == entName + "/display/set" ) {
                    DynamicJsonBuffer jsonBuffer( 200 );
                    JsonObject &      root = jsonBuffer.parseObject( msg );
                    if ( !root.success() ) {
                        DBG( "LCD display, set, on Receive: Invalid JSON received!" );
                        return;
                    }
                    int x = 0, y = 0;
                    x           = root["x"];
                    y           = root["y"];
                    String text = root["text"];
                    int    f    = 2;
                    f           = root["textsize"];
                    int cl      = root["clear"];
                    if ( cl != 0 )
                        poled->clearDisplay();
                    poled->setTextSize( f );
                    poled->setCursor( x, y );
                    poled->print( text );
                    bRedraw = true;
                }
            }
        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
