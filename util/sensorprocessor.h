// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

namespace meisterwerk {
    namespace util {

        class sensorprocessor {
            public:
            int           noVals = 0;
            int           smoothIntervall;
            int           pollTimeSec;
            double        sum = 0.0;
            double        eps;
            bool          first   = true;
            double        meanVal = 0;
            double        lastVal = -99999.0;
            unsigned long last;

            // average of smoothIntervall measurements
            // update sensor value, if newvalue differs by at least eps, or if pollTimeSec has
            // elapsed.
            sensorprocessor( int smoothIntervall = 5, int pollTimeSec = 60, double eps = 0.1 )
                : smoothIntervall{smoothIntervall}, pollTimeSec{pollTimeSec}, eps{eps} {
                last = millis();
            }

            // changes the value into a smoothed version
            // returns true, if sensor-value is a valid update
            // an update is valid, if the new value differs by at least eps from last last value,
            // or, if pollTimeSec secs have elapsed.
            bool filter( double *pvalue ) {
                meanVal = ( meanVal * noVals + ( *pvalue ) ) / ( noVals + 1 );
                if ( noVals < smoothIntervall ) {
                    ++noVals;
                }
                double delta = lastVal - meanVal;
                if ( delta < 0.0 ) {
                    delta = ( -1.0 ) * delta;
                }
                if ( delta > eps || first ) {
                    first   = false;
                    lastVal = meanVal;
                    *pvalue = meanVal;
                    last    = millis();
                    return true;
                } else {
                    if ( pollTimeSec != 0 ) {
                        if ( timebudget::delta( last, millis() ) > pollTimeSec * 1000L ) {
                            *pvalue = meanVal;
                            last    = millis();
                            lastVal = meanVal;
                            return true;
                        }
                    }
                }
                return false;
            }

            bool filter( long *plvalue ) {
                double tval = (double)*plvalue;
                bool   ret  = filter( &tval );
                if ( ret ) {
                    *plvalue = (long)tval;
                }
                return ret;
            }
        };
    } // namespace util
} // namespace meisterwerk