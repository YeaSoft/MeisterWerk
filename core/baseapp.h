// baseapp.h - The base application class
//
// This is the declaration of the base class for an
// embedded application.

#ifndef baseapp_h
#define baseapp_h

// dependencies
#include "scheduler.h"

namespace meisterwerk {
    namespace core {

        class baseapp {
            public:
            baseapp( String name ) {
                appName = name;
                _app    = this;
            }

            virtual void onSetup() {
            }

            virtual void onLoop() {
                sched.loop();
            }

            // members
            String    appName;
            scheduler sched;

            // general availability
            static baseapp *_app;
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

#endif
