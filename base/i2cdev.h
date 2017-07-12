
// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// hardware dependencies
#include <ESP8266WiFi.h>

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "../core/entity.h"
#include "../util/hextools.h"

namespace meisterwerk {
    namespace base {

        class i2cdev : public meisterwerk::core::entity {
            public:
            String i2ctype;

            i2cdev( String name, String i2cType )
                : meisterwerk::core::entity( name ), i2ctype{i2cType} {
            }

            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 50000 );
            }

            virtual void onSetup() override {
                DBG( "i2cdev pub/sub in setup" );
                subscribe( "i2cbus/online" );
                publish( "i2cbus/enum", "" );
            }

            virtual void onReceive( String topic, String msg ) override {
                if ( topic == "i2cbus/online" ) {
                    i2cSetup( msg );
                }
            }

            virtual void onInstantiate( String i2ctype, uint8_t address ) {
                DBG( "Your code should override this function and instantiate a " + i2ctype +
                     " device at address 0x" + meisterwerk::utils::hexByte( address ) );
            }

            void i2cSetup( String json ) {
                bool ok = false;
                DBG( "Received i2c-info: " + json );
                DynamicJsonBuffer jsonBuffer( 200 );
                JsonObject &      root = jsonBuffer.parseObject( json );
                if ( !root.success() ) {
                    DBG( "Invalid JSON received!" );
                    return;
                }
                JsonArray &devs  = root["i2cdevs"];
                JsonArray &ports = root["portlist"];
                for ( int i = 0; i < devs.size(); i++ ) {
                    String  dev     = devs[i];
                    uint8_t address = (uint8_t)ports[i];
                    DBG( i2ctype + "<->" + dev );
                    if ( dev == i2ctype ) {
                        onInstantiate( i2ctype, address );
                    }
                }
            }
        };
    } // namespace base
} // namespace meisterwerk
