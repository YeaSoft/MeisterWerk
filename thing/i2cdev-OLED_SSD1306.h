// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <Adafruit_SSD1306.h>

// dependencies
#include "../base/i2cdev.h"
#include "../core/entity.h"
#include "../util/hextools.h"

namespace meisterwerk {
    namespace thing {
        class i2cdev_OLED_SSD1306 : public meisterwerk::base::i2cdev {
            public:
            Adafruit_SSD1306 *poled;
            bool              pollDisplay = false;
            String            json;
            uint8_t           adr;
            uint8_t           instAddress;
            int               displayX, displayY;
            bool              bRedraw = false;

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

            virtual void setup() override {
                i2cdev::setup();
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
                DBG( "Instantiating OLED_SSD1306 device {" + i2ctype + "," + entName + "} at address 0x" +
                     meisterwerk::util::hexByte( address ) + ", " + String( displayY ) + "x" + String( displayX ) );
                adr   = address;
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
                subscribe( "display/get" );
                subscribe( "*/luminosity" );
                publish( entName + "/display" );
            }

            virtual void loop() override {
                if ( bRedraw )
                    poled->display();
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
                    float lx         = atof( root["luminosity"] );
                    int   oledBright = ( lx / 1000.0 * 256.0 );
                    if ( oledBright > 255 )
                        oledBright = 255;
                    // XXX: freaking adafruit lib cant set brightness!
                    // poled->setContrast( (unsigned char)oledBright );
                }
                if ( topic == "display/get" || topic == entName + "/display/get" ) {
                    publish( entName + "/display", "{\"type\":\"pixeldisplay\",\"x\":" + String( displayX ) +
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
