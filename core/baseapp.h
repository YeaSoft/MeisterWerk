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

            // mthods
            baseapp( String name = "app" ) : entity( name ) {
                _app = this;
            }

            virtual void onSetup() {
            }

            // there is no clash between baseapp:onLoop and entity;:onLoop
            // because of the number of parameters.
            virtual void onLoop() {
                sched.loop();
            }

            virtual void onGetState( JsonObject &request, JsonObject &response ) override {
            }

            virtual bool onSetState( JsonObject &request, JsonObject &response ) override {
                return false;
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
