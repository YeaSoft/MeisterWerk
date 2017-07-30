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

            void setlength( unsigned long length ) {
                beatLength = length;
                timerStart = millis();
            }

            // real metronome: tries to be synchrtonous with the real beat
            unsigned long beat() {
                unsigned long now   = millis();
                unsigned long delta = timebudget::delta( timerStart, now );
                if ( beatLength && delta >= beatLength ) {
                    timerStart = now - ( delta % beatLength );
                    return delta / beatLength;
                }
                return 0;
            }

            // watchdog style: the specified interval has passed
            unsigned long woof() {
                unsigned long now   = millis();
                unsigned long delta = timebudget::delta( timerStart, now );
                if ( beatLength && delta >= beatLength ) {
                    timerStart = now;
                    return delta / beatLength;
                }
                return 0;
            }
        };
    } // namespace util
} // namespace meisterwerk
