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

            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 50000 );
            }

            virtual void onRegister() override {
                pinMode( pin, INPUT );
                fromState  = digitalRead( pin ) == LOW;
                lastChange = micros();
            }

            virtual void onLoop( unsigned long ticker ) override {
                change( digitalRead( pin ) == LOW );
            }
        };
    }
}
