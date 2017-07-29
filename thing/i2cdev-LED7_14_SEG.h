// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <Adafruit_LEDBackpack.h>

// dependencies
#include "../base/i2cdev.h"
#include "../core/entity.h"
#include "../util/hextools.h"

namespace meisterwerk {
    namespace thing {
        class i2cdev_LED7_14_SEG : public meisterwerk::base::i2cdev {
            public:
            Adafruit_7segment * pled7;
            Adafruit_AlphaNum4 *pled14;
            bool                pollDisplay = false;
            uint8_t             address;
            uint8_t             segments;
            util::metronome     scroller;
            util::metronome     blinker;
            int                 ptext;
            int                 displayX   = 4;
            int                 displayY   = 1;
            int                 blinkMode  = 0;
            int                 blinkState = 0;
            String              json;

            i2cdev_LED7_14_SEG( String name, uint8_t address, uint8_t segments )
                : meisterwerk::base::i2cdev( name, "LED7_14_SEG", address ), scroller( 250 ),
                  blinker( 500 ), address{address}, segments{segments} {
            }
            ~i2cdev_LED7_14_SEG() {
                if ( pollDisplay ) {
                    pollDisplay = false;
                    if ( segments == 7 )
                        delete pled7;
                    else if ( segments == 14 )
                        delete pled14;
                }
            }

            virtual void setup() override {
            }

            virtual void onInstantiate( String i2ctype, uint8_t address ) override {
                DBG( "Instantiating LED7_14_SEG device at address 0x" + meisterwerk::util::hexByte( address ) +
                     ", segments: " + String( segments ) );
                if ( segments == 7 ) {
                    pled7 = new Adafruit_7segment();
                    pled7->begin( address );
                } else if ( segments == 14 ) {
                    pled14 = new Adafruit_AlphaNum4();
                    pled14->begin( address );
                }
                pollDisplay = true;
                subscribe( entName + "/display/set" );
                subscribe( entName + "/display/get" );
                publish( entName + "/display" );
            }

            void print( String text ) {
                String t;
                t = text;
                if ( segments == 7 ) {
                    while ( t.length() < 4 )
                        t += " ";
                    pled7->writeDigitNum( 0, t[0] - '0' );
                    pled7->writeDigitNum( 1, t[1] - '0' );
                    pled7->writeDigitNum( 3, t[2] - '0' );
                    pled7->writeDigitNum( 4, t[3] - '0' );
                    pled7->writeDisplay();
                }
                if ( segments == 14 ) {
                    while ( t.length() < 4 )
                        t += " ";
                    for ( int i = 0; i < 4; i++ ) {
                        pled14->writeDigitAscii( i, t[i] );
                    }
                    pled14->writeDisplay();
                }
            }

            virtual void loop() override {
                if ( pollDisplay ) {
                    if ( segments == 7 ) {
                        if ( blinkMode == 1 ) {
                            int nr = blinker.beat();
                            if ( nr > 0 ) {
                                blinkState += nr;
                                pled7->drawColon( blinkState % 2 );
                            }
                        }
                    }
                    if ( segments == 14 ) {
                    }
                    /*
                    if ( disptext.length() > 4 ) {
                        int d = scroller.beat();
                        if ( d > 0 ) {
                            ptext        = ( ptext + d ) % disptext.length();
                            String dtext = disptext.substring( ptext, ptext + 4 );
                            print( dtext );
                        }
                    }
                    */
                }
            }

            virtual void receive( const char *origin, const char *ctopic, const char *msg ) override {
                meisterwerk::base::i2cdev::receive( origin, ctopic, msg );
                String topic( ctopic );
                if ( topic == "*/display/get" || topic == entName + "/display/get" ) {
                    publish( entName + "/display", "{\"type\":\"textdisplay\",\"x\":" + String( displayX ) +
                                                       ",\"y\":" + String( displayY ) +
                                                       ",\"segments\":" + String( segments ) + "}" );
                }
                if ( topic == entName + "/display/set" ) {
                    DynamicJsonBuffer jsonBuffer( 200 );
                    JsonObject &      root = jsonBuffer.parseObject( msg );
                    if ( !root.success() ) {
                        DBG( "LED display, set, on Receive: Invalid JSON received!" );
                        return;
                    }
                    String text = root["text"];
                    blinkMode   = root["blink"];
                    print( text );
                }
            }
        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
