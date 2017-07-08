#ifndef baseapp_h
#define baseapp_h

namespace meisterwerk {
    namespace core {

        class baseapp {
            public:
            baseapp() {
                _app = this;
            }

            virtual void onSetup() {
            }

            virtual void onLoop() {
            }

            static baseapp *_app;
        };

        // initialization of static member
        baseapp *baseapp::_app = 0;

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
