// baseapp.h - The base application class
//
// This is the declaration of the base class for an
// embedded application.

#pragma once

// dependencies
#include "entity.h"
#include "scheduler.h"

namespace meisterwerk {
    namespace core {

        class baseapp : public entity {
            public:
            // static members
            static baseapp *_app;

            // members
            scheduler sched;

            // methods
            baseapp( String name = "app", unsigned long minMicroSecs = 0L, T_PRIO priority = PRIORITY_NORMAL )
                : entity( name ) {
                _app = this;
                // direct registration into the scheduler - always first entity
                // setup() callback will not be invoked from scheduler but
                // directly from arduino core setup()
                sched.registerEntity( this, minMicroSecs, priority, false );
            }
        };

        // initialization of static member
        baseapp *baseapp::_app = nullptr;

    } // namespace core
} // namespace meisterwerk

// application entry points
void setup() {
    // Call the application setup callback
    meisterwerk::core::baseapp::_app->setup();
}

void loop() {
    // Call the scheduler loop
    meisterwerk::core::baseapp::_app->sched.loop();
}
