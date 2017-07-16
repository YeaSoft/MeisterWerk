// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// dependencies
#include "../base/onoff.h"

namespace meisterwerk {
    namespace thing {

        class onoff_GPIO : public meisterwerk::base::onoff {
            public:
            uint8_t pin;

            onoff_GPIO( String name, uint8_t pin ) : meisterwerk::base::onoff( name ), pin{pin} {
            }

            virtual void onRegister() override {
                meisterwerk::base::onoff::onRegister();
                pinMode( pin, OUTPUT );
                digitalWrite( pin, HIGH );
            }

            virtual bool onSwitch( bool newstate ) override {
                digitalWrite( pin, newstate ? LOW : HIGH );
                return true;
            }

            virtual void onGetState( JsonObject &request, JsonObject &response ) override {
                meisterwerk::base::onoff::onGetState( request, response );
                response["type"] = response["type"].as<String>() + String( "/onoff-GPIO" );
                response["pin"]  = pin;
            }
        };
    }
}
