#ifndef gpiobutton_h
#define gpiobutton_h

#include "../base/button.h"

namespace meisterwerk {
    namespace thing {

        class gpiobutton : public meisterwerk::base::button {
            public:
            uint8_t pin;

            gpiobutton( String name, uint8_t _pin ) : meisterwerk::base::button( name ) {
                pin = _pin;
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

#endif
