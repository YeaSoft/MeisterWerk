// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

namespace meisterwerk {
    namespace util {

        class timebudget {
            private:
            unsigned long cnt     = 0;
            unsigned long val     = 0;
            unsigned long valMax  = 0;
            unsigned long valFine = 0;
            unsigned long valSnap = 0;

            public:
            static unsigned long delta( const unsigned long then, const unsigned long now ) {
                return now > then ? now - then : ( (unsigned long)-1 ) - then + now;
            }

            void inc( const unsigned long inc ) {
                cnt++;
                valFine += inc; // XXX: needs further work.
                if ( valFine > 1000000000L ) {
                    val += ( valFine / 1000 );
                    valFine = valFine % 1000;
                }
                if ( inc > valMax ) {
                    valMax = inc;
                }
            }

            void deltainc( const unsigned long then, const unsigned long now ) {
                inc( now >= then ? now - then : ( (unsigned long)-1 ) - then + now );
            }

            void snap() {
                valSnap = micros();
            }

            void shot() {
                deltainc( valSnap, micros() );
                valSnap = micros();
            }

            unsigned long getms() const {
                return val + ( valFine / 1000 );
            }

            unsigned long getmaxus() const {
                return valMax;
            }

            unsigned long getcount() const {
                return cnt;
            }
        };
    } // namespace util
} // namespace meisterwerk
