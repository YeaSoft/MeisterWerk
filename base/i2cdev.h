
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
            private:
            String i2ctype;
            bool   isInstantiated = false;

            public:
            uint8_t address;
            i2cdev( String name, String i2cType, uint8_t address )
                : meisterwerk::core::entity( name ), i2ctype{i2cType}, address{address} {
                DBG( "Constr:" + i2ctype );
            }

            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 50000 );
            }

            virtual void onRegister() override {
                // DBG( "i2cdev pub/sub in setup" );
                subscribe( "i2cbus/devices" );
                publish( "i2cbus/devices/get", "" );
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                if ( topic == "i2cbus/devices" ) {
                    if ( !isInstantiated )
                        i2cSetup( msg );
                }
            }

            virtual void onInstantiate( String i2ctype, uint8_t address ) {
                DBG( "Your code should override this function and instantiate a " + i2ctype +
                     " device at address 0x" + meisterwerk::util::hexByte( address ) );
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
                JsonArray &devs = root["devices"];
                for ( int i = 0; i < devs.size(); i++ ) {
                    JsonObject &dev = devs[i];
                    for ( auto obj : dev ) {
                        // DBG( String( obj.key ) + "<->" + i2ctype );
                        if ( i2ctype == obj.key && address == obj.value ) {
                            onInstantiate( i2ctype, address );
                            isInstantiated = true;
                        }
                    }
                }
            }
        };
    } // namespace base
} // namespace meisterwerk
