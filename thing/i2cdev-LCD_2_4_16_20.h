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
            uint8_t adr;
            uint8_t instAddress;
            int     displayX, displayY;

            i2cdev_LCD_2_4_16_20( String name, uint8_t address, int y, int x )
                : meisterwerk::base::i2cdev( name, "LCD_2_4_16_20", address ),
                  instAddress{address}, displayY{y}, displayX{x} {
            }
            ~i2cdev_LCD_2_4_16_20() {
                if ( pollDisplay ) {
                    pollDisplay = false;
                    delete plcd;
                }
            }

            bool registerEntity() {
                // 5sec sensor checks
                bool ret = meisterwerk::base::i2cdev::registerEntity( 5000000 );
                return ret;
            }

            virtual void onInstantiate( String i2ctype, uint8_t address ) override {
                if ( address != instAddress ) {
                    return; // not me.
                }
                if ( pollDisplay ) {
                    DBG( "Tried to re-instanciate object: {" + i2ctype + "," + entName + "} at address 0x" +
                         meisterwerk::util::hexByte( address ) + "Display: " + String( displayY ) + "x" +
                         String( displayX ) );
                    return;
                }
                DBG( "Instantiating LCD_2_4_16_20 device {" + i2ctype + "," + entName + "} at address 0x" +
                     meisterwerk::util::hexByte( address ) + ", " + String( displayY ) + "x" + String( displayX ) );
                adr = address;
                if ( displayY == 2 && displayX == 16 ) {
                    plcd = new LiquidCrystal_PCF8574( address ); // 0: default address;
                    plcd->begin( 16, 2 );
                } else if ( displayY == 4 && displayX == 20 ) {
                    plcd = new LiquidCrystal_PCF8574( address ); // 0: default address;
                    plcd->begin( 20, 4 );
                } else {
                    DBG( "Uknown display size, cannot instantiate LCD entity: ERROR" );
                    return;
                }
                plcd->setBacklight( 255 );
                plcd->home();
                plcd->clear();
                pollDisplay = true;
                subscribe( entName + "/display/set" );
                subscribe( entName + "/display/get" );
                publish( entName + "/display" );
            }

            int          l = 0;
            virtual void onLoop( unsigned long ticker ) override {
                if ( pollDisplay ) {
                    l++;
                }
            }

            virtual void onReceive( const char *origin, const char *ctopic, const char *msg ) override {
                meisterwerk::base::i2cdev::onReceive( origin, ctopic, msg );
                String topic( ctopic );
                if ( topic == "*/display/get" || topic == entName + "/display/get" ) {
                    publish( entName + "/display",
                             "{\"type\":\"textdisplay\",\"x\":" + String( displayX ) + ",\"y\":" + String( displayY ) +
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
                    int    cl   = root["clear"];
                    if ( cl != 0 )
                        plcd->clear();
                    plcd->setCursor( x, y );
                    plcd->print( text );
                }
            }
        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
