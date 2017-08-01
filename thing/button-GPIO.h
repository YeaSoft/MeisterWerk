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

            button_GPIO( String name, uint8_t pin ) : meisterwerk::base::button( name, 50000 ), pin{pin} {
                // default sample rate: 50ms
            }

            virtual void setup() override {
                button::setup();
                pinMode( pin, INPUT );
                fromState = digitalRead( pin ) == LOW;
                lastChange.start();
            }

            virtual void loop() override {
                change( digitalRead( pin ) == LOW );
            }
        };
    } // namespace thing
} // namespace meisterwerk
