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

        class eggtimer {
            private:
            unsigned long duration;
            unsigned long timerStart;

            public:
            eggtimer() {
                duration   = 0;
                timerStart = 0;
            }

            eggtimer( const eggtimer &other ) {
                duration   = other.duration;
                timerStart = other.timerStart;
            }

            eggtimer( unsigned long duration ) {
                init( duration );
            }

            operator unsigned long() const {
                return duration;
            }

            void init( unsigned long newduration ) {
                duration   = newduration;
                timerStart = millis();
            }

            bool isexpired() {
                if ( duration == 0 ) {
                    return true;
                }
                unsigned long check = millis();
                unsigned long delta = timebudget::delta( timerStart, check );
                if ( delta > duration ) {
                    // expired
                    duration = 0;
                    return true;
                }
                duration -= delta;
                timerStart = check;
                return false;
            }

            bool isrunning() {
                return !isexpired();
            }

            unsigned int getduration() const {
                return duration;
            }
        };
    } // namespace util
} // namespace meisterwerk