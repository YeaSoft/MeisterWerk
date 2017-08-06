// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// external libraries
#include <ArduinoJson.h>

// dependencies
#include "msgtime.h"
#include "sensorprocessor.h"

namespace meisterwerk {
    namespace util {

        class sensorvalue : public sensorprocessor {
            public:
            enum result { INVALID, VALID, CHANGED };
            double      valueLast;
            const char *valueName;
            time_t      valueTime;
            int         validSecs;

            // average of smoothIntervall measurements
            // update sensor value, if newvalue differs by at least eps, or if pollTimeSec has
            // elapsed.
            sensorvalue( const char *name, int validitySecs = 0, int smoothIntervall = 5, int pollTimeSec = 60,
                         double eps = 0.1 )
                : sensorprocessor( smoothIntervall, pollTimeSec, eps ) {
                valueLast = NAN;
                valueName = name;
                validSecs = 0;
            }

            result set( double value ) {
                if ( isnan( value ) ) {
                    return INVALID;
                } else {
                    if ( filter( &value ) ) {
                        valueLast = value;
                        valueTime = now();
                        return CHANGED;
                    }
                    return VALID;
                }
            }

            bool prepare( JsonObject &data, const char *sensorType = nullptr, bool withTime = true ) {
                if ( isvalid() ) {
                    data[valueName] = valueLast;
                    data["age"]     = timebudget::delta( last, millis() );
                    if ( sensorType ) {
                        data["sensortype"] = sensorType;
                    }
                    if ( withTime ) {
                        if ( timeStatus() != timeNotSet ) {
                            data["time"] = getTime();
                        }
                    }
                    return true;
                }
                return false;
            }

            double get() const {
                return valueLast;
            }

            String getTime() const {
                return util::msgtime::time_t2ISO( valueTime );
            }

            const char *getName() const {
                return valueName;
            }

            bool isvalid() {
                if ( validSecs && !isnan( valueLast ) ) {
                    if ( now() - valueTime > validSecs ) {
                        // value is outdated
                        reset();
                        return false;
                    }
                }
                return !isnan( valueLast );
            }

            void reset() {
                sensorprocessor::reset();
                valueLast = NAN;
                // we do not reset the last value time, since it may be interesting
                // also if the reading is invalid due to age
                // valueTime = 0;
            }
        };
    } // namespace util
} // namespace meisterwerk