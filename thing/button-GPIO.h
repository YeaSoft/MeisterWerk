// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// dependencies
#include "../base/button.h"

namespace meisterwerk {
    namespace thing {

        class button_GPIO : public meisterwerk::base::button {
            public:
            uint8_t pin;

            button_GPIO( String name, uint8_t pin ) : meisterwerk::base::button( name ), pin{pin} {
            }

            bool registerEntity( unsigned long minMicroSecs = 50000,
                                 unsigned int  priority     = meisterwerk::core::scheduler::PRIORITY_NORMAL ) {
                // default sample rate: 50ms
                return meisterwerk::base::button::registerEntity( minMicroSecs, priority );
            }

            virtual void onRegister() override {
                button::onRegister();
                pinMode( pin, INPUT );
                fromState = digitalRead( pin ) == LOW;
                lastChange.start();
            }

            virtual void onLoop( unsigned long ticker ) override {
                change( digitalRead( pin ) == LOW );
            }

            virtual void onGetState( JsonObject &request, JsonObject &response ) override {
                meisterwerk::base::button::onGetState( request, response );
                response["type"] = response["type"].as<String>() + String( "/button-GPIO" );
                reposnse["pin"]  = pin;
            }
        };
    }
}
