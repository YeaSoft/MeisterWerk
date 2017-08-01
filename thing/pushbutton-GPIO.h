// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// dependencies
#include "../base/pushbutton.h"

namespace meisterwerk {
    namespace thing {

        class pushbutton_GPIO : public meisterwerk::base::pushbutton {
            public:
            uint8_t pin;

            pushbutton_GPIO( String name, uint8_t pin, unsigned int minLongMs = 0, unsigned int minExtraLongMs = 0 )
                : meisterwerk::base::pushbutton( name, minLongMs, minExtraLongMs, 50000 ), pin{pin} {
                // default sample rate: 50ms
            }

            virtual void setup() override {
                meisterwerk::base::pushbutton::setup();
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
