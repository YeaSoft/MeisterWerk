// topic.h - The Topic class
//
// This class is derived from String and
// implements some useful helpers for handling
// MQTT topics

#pragma once

// dependencies

namespace meisterwerk {
    namespace core {

        class Topic : public String {
            public:
            Topic( const char *cstr = "" ) : String( cstr ) {
            }
            Topic( const String &str ) : String( str ) {
            }
            Topic( const __FlashStringHelper *str ) : String( str ) {
            }
#ifdef __GXX_EXPERIMENTAL_CXX0X__
            Topic( String &&rval ) : String( rval ) {
            }
            Topic( StringSumHelper &&rval ) : String( rval ) {
            }
#endif
            explicit Topic( char c ) : String( c ) {
            }
            explicit Topic( unsigned char value, unsigned char base = 10 ) : String( value, base ) {
            }
            explicit Topic( int value, unsigned char base = 10 ) : String( value, base ) {
            }
            explicit Topic( unsigned int value, unsigned char base = 10 ) : String( value, base ) {
            }
            explicit Topic( long value, unsigned char base = 10 ) : String( value, base ) {
            }
            explicit Topic( unsigned long value, unsigned char base = 10 ) : String( value, base ) {
            }
            explicit Topic( float value, unsigned char decimalPlaces = 2 ) : String( value, decimalPlaces ) {
            }
            explicit Topic( double value, unsigned char decimalPlaces = 2 ) : String( value, decimalPlaces ) {
            }

            bool match( String &mask ) const {
                return mqttmatch( c_str(), mask.c_str() );
            }

            bool match( const char *mask ) const {
                return mqttmatch( c_str(), mask );
            }

            String getfirst() const {
                char *pPtr = strchr( buffer, '/' );
                if ( pPtr ) {
                    char cTemp = *pPtr;
                    *pPtr      = '\0';
                    String out = buffer;
                    *pPtr      = cTemp;
                    return out;
                }
                return c_str();
            }

            String getnext() const {
                char *pStart = strchr( buffer, '/' );
                if ( pStart ) {
                    ++pStart;
                    char *pEnd = strchr( pStart, '/' );
                    if ( pEnd ) {
                        char cTemp = *pEnd;
                        *pEnd      = '\0';
                        String out = pStart;
                        *pEnd      = cTemp;
                        return out;
                    }
                    return pStart;
                }
                return "";
            }

            String &format( const char *format, ... ) {
                va_list argList;
                va_start( argList, format );
                String &ret = formatv( format, argList );
                va_end( argList );
                return ret;
            }

            String &format( const String &format, ... ) {
                va_list argList;
                va_start( argList, format );
                String &ret = formatv( format.c_str(), argList );
                va_end( argList );
                return ret;
            }

            String &formatv( const char *format, va_list argList ) {
                int reslen = vsnprintf( buffer, capacity, format, argList );
                if ( reslen >= capacity ) {
                    if ( reserve( reslen + 1 ) ) {
                        vsnprintf( buffer, capacity, format, argList );
                    } else {
                        invalidate();
                    }
                }
                return *this;
            }

            String &formatv( const String &format, va_list argList ) {
                int reslen = vsnprintf( buffer, capacity, format.c_str(), argList );
                if ( reslen >= capacity ) {
                    if ( reserve( reslen + 1 ) ) {
                        vsnprintf( buffer, capacity, format.c_str(), argList );
                    } else {
                        invalidate();
                    }
                }
                return *this;
            }

            public:
            static bool mqttmatch( const String &pub, const String &sub ) {
                return mqttmatch( pub.c_str(), sub.c_str() );
            }

            static bool mqttmatch( const char *pub, const char *sub ) {
                // compares publish and subscribe topics.
                // subscriptions can contain the MQTT wildcards '#' and '+'.
                if ( pub == sub )
                    return true;
                int lp = strlen( pub );
                int ls = strlen( sub );

                bool wPos = true; // sub wildcard is legal now
                int  ps   = 0;
                for ( int pp = 0; pp < lp; pp++ ) {
                    // if ( pp >= ls || ps > ls ) {
                    //    DBG( "Pub more spec than sub: " + String( pp ) + "," + String( ps ) );
                    //    return false; // Pub is more specific than sub
                    // }
                    if ( pub[pp] == '+' || pub[pp] == '#' ) {
                        DBG( "Bad wildcard in pub" );
                        return false; // Illegal wildcards in pub
                    }
                    if ( wPos ) {
                        wPos = false;
                        if ( sub[ps] == '#' ) {
                            if ( ps == ls - 1 ) {
                                DBG( "# ending (+)" );
                                return true;
                            } else {
                                DBG( "# followed by stuff! (-)" );
                                return false; // In sub, # must not be followed by anything else
                            }
                        }
                        if ( sub[ps] == '+' ) {
                            while ( pp < lp && pub[pp] != '/' )
                                ++pp;
                            ++ps;
                            if ( pp == lp ) {
                                if ( ps == ls ) {
                                    DBG( "End in + compare (+)" );
                                    return true;
                                } else if ( !strcmp( &sub[ps], "/#" ) ) {
                                    DBG( "End in + and /# (+)" );
                                    return true;
                                }
                            }
                        }
                    } else {
                        if ( sub[ps] == '+' || sub[ps] == '#' ) {
                            DBG( "bad wildcard position (-)" );
                            return false; // Illegal wildcard-position
                        }
                    }
                    if ( pub[pp] != sub[ps] && strcmp( &sub[ps], "/#" ) ) {
                        DBG( "char mismatch (-)" );
                        return false;
                    }
                    if ( pub[pp] == '/' )
                        wPos = true;
                    if ( pp == lp - 1 ) {
                        if ( ps == ls - 1 ) {
                            DBG( "End pub/sub (+)" );
                            return true;
                        }
                        if ( !strcmp( &sub[ps + 1], "/#" ) || !strcmp( &sub[ps + 1], "#" ) ||
                             !strcmp( &sub[ps + 1], "+" ) ) {
                            DBG( "End followed by sub '/#' is ok (+)" );
                            return true;
                        }
                        DBG( "Overlap on sub (-)" );
                        return false;
                    }
                    ++ps;
                }
                if ( ps == ls ) {
                    DBG( "Loop end: Sub finished (+)" );
                    return true;
                } else {
                    DBG( "Loop end: Sub not finished (-)" );
                    return false;
                }
            }

        }; // namespace core
    }      // namespace core
} // namespace meisterwerk
