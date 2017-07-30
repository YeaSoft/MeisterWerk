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

            bool match( String &mask ) const {
                return mqttmatch( c_str(), mask.c_str() );
            }

            bool match( const char *mask ) const {
                return mqttmatch( c_str(), mask );
            }

            String getFirst() const {
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

            String getNext() const {
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
                    if ( pp >= ls || ps >= ls )
                        return false; // Pub is more specific than sub
                    if ( pub[pp] == '+' || pub[pp] == '#' )
                        return false; // Illegal wildcards in pub
                    if ( wPos ) {
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
                        }
                        wPos = false;
                    } else {
                        if ( sub[ps] == '+' || sub[ps] == '#' ) {
                            DBG( "bad wildcard position (-)" );
                            return false; // Illegal wildcard-position
                        }
                    }
                    if ( pub[pp] != sub[ps] ) {
                        DBG( "char mismatch (-)" );
                        return false;
                    }
                    if ( pub[pp] == '/' )
                        wPos = true;
                    if ( pp == lp - 1 ) {
                        if ( ps == ls - 1 )
                            return true;
                        if ( !strcmp( &sub[ps + 1], "/#" ) )
                            return true;
                    }
                    ++ps;
                }
                return true;
            }

        }; // namespace core
    }      // namespace core
} // namespace meisterwerk
