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
                : meisterwerk::base::pushbutton( name, minLongMs, minExtraLongMs ), pin{pin} {
            }

            bool registerEntity( unsigned long minMicroSecs = 50000,
                                 unsigned int  priority     = meisterwerk::core::scheduler::PRIORITY_NORMAL ) {
                // default sample rate: 50ms
                return meisterwerk::base::pushbutton::registerEntity( minMicroSecs, priority );
            }

            virtual void onRegister() override {
                meisterwerk::base::pushbutton::onRegister();
                pinMode( pin, INPUT );
                fromState = digitalRead( pin ) == LOW;
                lastChange.start();
            }

            virtual void loop() override {
                change( digitalRead( pin ) == LOW );
            }

            virtual void onGetState( JsonObject &request, JsonObject &response ) override {
                meisterwerk::base::pushbutton::onGetState( request, response );
                response["type"] = response["type"].as<String>() + String( "/pushbutton-GPIO" );
                response["pin"]  = pin;
            }
        };
    } // namespace thing
} // namespace meisterwerk
