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
            metronome( unsigned long _beatLength = 0 ) {
                timerStart = millis();
                beatLength = _beatLength;
            }

            bool beat() {
                unsigned int now = millis();
                if ( beatLength && timebudget::delta( timerStart, now ) >= beatLength ) {
                    timerStart = now;
                    return true;
                }
                return false;
            }
        };
    } // namespace util
} // namespace meisterwerk