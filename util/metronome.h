// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// dependencies
#include "timebudget.h"

namespace meisterwerk {
    namespace util {

        class metronome {
            private:
            unsigned long timerStart;
            unsigned long beatLength;

            public:
            metronome( unsigned long beatLength = 0 ) : beatLength{beatLength} {
                timerStart = millis();
            }

            operator unsigned long() const {
                return beatLength;
            }

            unsigned long getlength() const {
                return beatLength;
            }

            unsigned long beat() {
                unsigned int now   = millis();
                unsigned int delta = timebudget::delta( timerStart, now );
                if ( beatLength && delta >= beatLength ) {
                    timerStart = now - ( delta % beatLength );
                    return delta / beatLength;
                }
                return 0;
            }
        };
    } // namespace util
} // namespace meisterwerk