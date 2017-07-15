
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
            String  i2ctype;
            bool    isInstantiated = false;
            uint8_t address;

            public:
            i2cdev( String name, String i2cType, uint8_t address )
                : meisterwerk::core::entity( name ), i2ctype{i2cType}, address{address} {
                DBG( "Constr:" + i2ctype );
            }

            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 50000 );
            }

            virtual void onRegister() override {
                // DBG( "i2cdev pub/sub in setup" );
                subscribe( "i2cbus/online" );
                publish( "i2cbus/enum", "" );
            }

            virtual void onReceive( String origin, String topic, String msg ) override {
                if ( topic == "i2cbus/online" ) {
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
                JsonArray &devs  = root["i2cdevs"];
                JsonArray &ports = root["portlist"];
                for ( int i = 0; i < devs.size(); i++ ) {
                    String  dev  = devs[i];
                    uint8_t addr = (uint8_t)ports[i];
                    // DBG( dev + "<->" + i2ctype );
                    if ( dev == i2ctype && addr == address ) {
                        onInstantiate( i2ctype, addr );
                        isInstantiated = true;
                    }
                }
            }
        };
    } // namespace base
} // namespace meisterwerk
