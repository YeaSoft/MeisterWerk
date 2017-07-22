// crypt.h - A helper for symmetric encrytpion
// based on XXTEA algorithm.

#pragma once
#include <ArduinoJson.h>
#include <Time.h>
#include <Timezone.h>

namespace meisterwerk {
    namespace util {
        static TimeChangeRule CEST = {"CEST", Last, Sun,
                                      Mar,    2,    120}; // Central European Summer Time
        static TimeChangeRule CET = {"CET ", Last, Sun,
                                     Oct,    3,    60}; // Central European Standard Time
        static Timezone CE( CEST, CET );

        class msgtime {
            public:
            // Central European Time (Frankfurt, Paris) // XXX: move to flash file system
            static time_t time_t2local( time_t utc ) {
                return CE.toLocal( utc ); // XXX: select time zone
            }

            static time_t ISO2time_t( String iso ) { // ISO: 2017-07-18T17:32:50Z
                time_t t;
                if ( iso.length() != 20 ) {
                    DBG( "Invalid ISO time legnth: " + iso );
                    return 0;
                }
                if ( iso[19] != 'Z' ) {
                    DBG( "Unsupported time zone: " + iso );
                    return 0;
                }
                TimeElements tt;
                tt.Year   = atoi( iso.substring( 0, 4 ).c_str() ) - 1970;
                tt.Month  = atoi( iso.substring( 5, 7 ).c_str() );
                tt.Day    = atoi( iso.substring( 8, 10 ).c_str() );
                tt.Hour   = atoi( iso.substring( 11, 13 ).c_str() );
                tt.Minute = atoi( iso.substring( 14, 16 ).c_str() );
                tt.Second = atoi( iso.substring( 17, 19 ).c_str() );
                t         = makeTime( tt );
                return t;
            }
            static String time_t2ISO( time_t t ) {
                TimeElements tt;
                breakTime( t, tt );
                char ISO[21];
                memset( ISO, 0, 21 );
                sprintf( ISO, "%04d-%02d-%02dT%02d:%02d:%02dZ", tt.Year + 1970, tt.Month, tt.Day,
                         tt.Hour, tt.Minute, tt.Second );
                return String( ISO );
            }
        };
    } // namespace util
} // namespace meisterwerk
