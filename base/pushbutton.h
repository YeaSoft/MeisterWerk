
#ifndef pushbutton_h
#define pushbutton_h

// dependencies
#include "button.h"

namespace meisterwerk {
    namespace base {
        class pushbutton : public button {
            public:
            unsigned int minLongMs;
            unsigned int minExtraLongMs;

            pushbutton( String name, unsigned int _minLongMs = 0, unsigned int _minExtraLongMs = 0 )
                : button( name ) {
                minLongMs      = _minLongMs;
                minExtraLongMs = _minExtraLongMs;
            }

            virtual void onChange( bool toState, unsigned long duration ) {
                if ( toState ) {
                    publish( entName + "/press", String( duration / 1000 ) );
                } else {
                    if ( minExtraLongMs && duration / 1000 > minExtraLongMs ) {
                        publish( entName + "/extralong", String( duration / 1000 ) );
                    } else if ( minLongMs && duration / 1000 > minLongMs ) {
                        publish( entName + "/long", String( duration / 1000 ) );
                    } else {
                        publish( entName + "/short", String( duration / 1000 ) );
                    }
                }
            }
        };
    }
}

#endif
