#ifndef timebudget_h
#define timebudget_h

namespace meisterwerk {
    namespace util {
        class timebudget {
            public:
            unsigned long cnt     = 0;
            unsigned long val     = 0;
            unsigned long valMax  = 0;
            unsigned long valFine = 0;
            unsigned long valSnap = 0;

            static unsigned long delta( const unsigned long then, const unsigned long now ) {
                return now > then ? now - then : ( (unsigned long)-1 ) - then + now;
            }

            void inc( const unsigned long inc ) {
                cnt++;
                valFine += inc;
                if ( valFine > 1000000000L ) {
                    val += ( valFine / 1000 );
                    valFine = 0;
                }
                if ( inc > valMax ) {
                    valMax = inc;
                }
            }

            void deltainc( const unsigned long then, const unsigned long now ) {
                inc( now > then ? now - then : ( (unsigned long)-1 ) - then + now );
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
    }
}

#endif
