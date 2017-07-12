// baseapp.h - The base application class
//
// This is the declaration of the base class for an
// embedded application.

#pragma once

// dependencies
#include "scheduler.h"

namespace meisterwerk {
    namespace core {

        class baseapp {
            public:
            // static members
            static baseapp *_app;

            // members
            String    appName;
            scheduler sched;

            // mthods
            baseapp( String name ) {
                appName = name;
                _app    = this;
            }

            virtual void onSetup() {
            }

            virtual void onLoop() {
                sched.loop();
            }
        };

        // initialization of static member
        baseapp *baseapp::_app = nullptr;

    } // namespace core
} // namespace meisterwerk

// application entry points
void setup() {
    meisterwerk::core::baseapp::_app->onSetup();
}

void loop() {
    meisterwerk::core::baseapp::_app->onLoop();
}
