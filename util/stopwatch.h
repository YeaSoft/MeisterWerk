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

        class stopwatch {
            private:
            unsigned long timerStart;

            public:
            stopwatch() {
                timerStart = millis();
            }

            stopwatch( const stopwatch &other ) {
                timerStart = other.timerStart;
            }

            operator unsigned long() const {
                return timebudget::delta( timerStart, millis() );
            }

            void start() {
                timerStart = millis();
            }

            unsigned long getloop() {
                unsigned long check = millis();
                unsigned long delta = timebudget::delta( timerStart, check );
                timerStart          = check;
                return delta;
            }

            unsigned int getduration() const {
                return timebudget::delta( timerStart, millis() );
            }
        };
    } // namespace util
} // namespace meisterwerk