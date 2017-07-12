#ifndef gpiopushbutton_h
#define gpiopushbutton_h

#include "../base/pushbutton.h"

namespace meisterwerk {
    namespace thing {

        class gpiopushbutton : public meisterwerk::base::pushbutton {
            public:
            uint8_t pin;

            gpiopushbutton( String name, uint8_t _pin, unsigned int _minLongMs = 0,
                            unsigned int _minExtraLongMs = 0 )
                : meisterwerk::base::pushbutton( name, _minLongMs, _minExtraLongMs ) {
                pin = _pin;
            }

            bool registerEntity() {
                return meisterwerk::core::entity::registerEntity( 50000 );
            }

            virtual void onSetup() override {
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
