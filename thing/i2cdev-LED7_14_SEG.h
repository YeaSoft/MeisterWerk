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
            Adafruit_7segment * pled7;
            Adafruit_AlphaNum4 *pled14;
            bool                pollDisplay = false;
            uint8_t             address;
            uint8_t             segments;
            util::metronome     scroller;
            String              disptext;
            int                 ptext;
            // meisterwerk::util::sensorprocessor tempProcessor, pressProcessor;
            String json;

            i2cdev_LED7_14_SEG( String name, uint8_t address, uint8_t segments )
                : meisterwerk::base::i2cdev( name, "LED7_14_SEG", address ),
                  scroller( 250 ), address{address}, segments{segments} {
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

            bool registerEntity() {
                // 5sec sensor checks
                bool ret = meisterwerk::core::entity::registerEntity( 25000 );
                return ret;
            }

            virtual void onInstantiate( String i2ctype, uint8_t address ) override {
                // String sa = meisterwerk::util::hexByte( address );
                DBG( "Instantiating LED7_14_SEG device at address 0x" +
                     meisterwerk::util::hexByte( address ) );
                if ( segments == 7 ) {
                    pled7 = new Adafruit_7segment();
                    pled7->begin( address );
                } else if ( segments == 14 ) {
                    pled14 = new Adafruit_AlphaNum4();
                    pled14->begin( address );
                }
                DBG( "Instance " + String( segments ) + "-LED on." );
                pollDisplay = true;
                subscribe( entName + "/config" );
                subscribe( "textdisplay/enum" );
                subscribe( entName + "/display" );
                publish( entName + "/textdisplay" );
            }

            void print( String text ) {
                String t;
                t = text;
                while ( t.length() < 4 )
                    t += " ";
                for ( int i = 0; i < 4; i++ ) {
                    pled14->writeDigitAscii( i, t[i] );
                }
                pled14->writeDisplay();
            }

            virtual void onLoop( unsigned long ticker ) override {
                if ( pollDisplay ) {
                    if ( disptext.length() > 4 ) {
                        int d = scroller.beat();
                        if ( d > 0 ) {
                            ptext        = ( ptext + d ) % disptext.length();
                            String dtext = disptext.substring( ptext, ptext + 4 );
                            print( dtext );
                        }
                    }
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
                    DynamicJsonBuffer jsonBuffer( 200 );
                    JsonObject &      root = jsonBuffer.parseObject( msg );
                    if ( !root.success() ) {
                        DBG( "Invalid JSON received!" );
                        return;
                    }
                    disptext = root["text"].as<char *>();
                    while ( disptext.length() < 4 )
                        disptext += " ";
                    if ( disptext.length() <= 4 ) {
                        print( disptext );
                    }
                }
            }

            void config( String msg ) {
                // XXX: do things
            }
        }; // namespace thing
    }      // namespace thing
} // namespace meisterwerk
