// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <LiquidCrystal_PCF8574.h>

// dependencies
#include "../base/i2cdev.h"
#include "../core/entity.h"
#include "../util/hextools.h"

namespace meisterwerk {
    namespace thing {
        class i2cdev_LCD_2_4_16_20 : public meisterwerk::base::i2cdev {
            public:
            LiquidCrystal_PCF8574 *plcd;
            bool                   pollDisplay = false;
            String                 json;
            uint8_t                adr;
            uint8_t                instAddress;
            int                    displayX, displayY;

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

            virtual void setup() override {
                i2cdev::setup();
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
                subscribe( "*/luminosity" );
                publish( entName + "/display" );
            }

            virtual void loop() override {
                if ( pollDisplay ) {
                }
            }

            virtual void receive( const char *origin, const char *ctopic, const char *msg ) override {
                meisterwerk::base::i2cdev::receive( origin, ctopic, msg );
                String topic( ctopic );
                int    p  = topic.indexOf( "/" );
                String t1 = topic.substring( p + 1 );
                if ( t1 == "luminosity" ) {
                    DBG( "Luminosity msg: " + String( msg ) );
                    DynamicJsonBuffer jsonBuffer( 200 );
                    JsonObject &      root = jsonBuffer.parseObject( msg );
                    if ( !root.success() ) {
                        DBG( "Oled/luminosity: Invalid JSON received: " + String( msg ) );
                        return;
                    }
                    float lx        = atof( root["luminosity"] );
                    int   lcdBright = ( lx / 1000.0 * 256.0 );
                    if ( lcdBright > 255 )
                        lcdBright = 255;
                    if ( lcdBright < 1 )
                        lcdBright = 1;
                    plcd->setBacklight( lcdBright );
                }

                if ( topic == "*/display/get" || topic == entName + "/display/get" ) {
                    publish( entName + "/display", "{\"type\":\"textdisplay\",\"x\":" + String( displayX ) +
                                                       ",\"y\":" + String( displayY ) + "}" );
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
