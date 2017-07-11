
#ifndef button_h
#define button_h

// dependencies
#include "../core/entity.h"

namespace meisterwerk {
    namespace base {
        class button : public meisterwerk::core::entity {
            public:
            bool          fromState;
            unsigned long lastChange;

            button( String name ) : meisterwerk::core::entity( name ) {
                fromState  = false;
                lastChange = 0;
            }

            virtual void onChange( bool toState, unsigned long duration ) {
                // fire a message
                if ( toState ) {
                    // press
                    publish( entName + "/press", String( duration / 1000 ) );
                } else {
                    // release
                    publish( entName + "/release", String( duration / 1000 ) );
                }
            }

            virtual void press() {
                change( true );
            }

            virtual void release() {
                change( false );
            }

            virtual void change( bool toState ) {
                if ( fromState != toState ) {
                    unsigned long last     = micros();
                    unsigned long duration = last > lastChange
                                                 ? last - lastChange
                                                 : ( (unsigned long)-1 ) - lastChange + last;
                    lastChange = last;
                    fromState  = toState;
                    // handle state change
                    onChange( toState, duration );
                }
            }
        };
    }
}

#endif
