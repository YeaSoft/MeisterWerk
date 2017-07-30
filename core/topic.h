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
            static bool mqttmatch( String &s1, String &s2 ) {
                return mqttmatch( s1.c_str(), s2.c_str() );
            }

            static bool mqttmatch( const char *s1, const char *s2 ) {
                // compares topic-paths <subtopic>/<subtopic/...
                // the compare is symmetric, s1==s2 <=> s2==s1.
                // subtopic can be <chars> or <chars>+'*', '*' must be last char of a subtopic.
                // '*' acts within only the current subtopic. Exception: a '*' as last character of
                // a topic-path
                //    matches all deeper subptopics:  a*==a/b/c/d/e, but a*/c1!=a1/b1/c1
                // Samples:   abc/def/ghi == */de*/*, abc/def!=abc, ab*==abc, a*==a/b/c/d
                //    a/b*==a/b/c/d/e, a/b*/d!=a/b/c/d
                if ( s1 == s2 )
                    return true;
                int l1 = strlen( s1 );
                int l2 = strlen( s2 );
                int l;
                if ( l1 < l2 )
                    l = l2;
                else
                    l  = l1;
                int p1 = 0, p2 = 0;
                for ( int i = 0; i < l; l++ ) {
                    if ( ( p1 > l1 ) || ( p2 > l2 ) )
                        return false;
                    if ( s1[p1] == s2[p2] ) {
                        ++p1;
                        ++p2;
                        if ( ( p1 == l1 ) && ( p2 == l2 ) )
                            return true;
                        continue;
                    }
                    if ( ( s1[p1] != '*' ) && ( s2[p2] != '*' ) )
                        return false;
                    if ( s1[p1] == '*' ) {
                        ++p1;
                        if ( p1 == l1 )
                            return true;
                        if ( s1[p1] != '/' )
                            return false;
                        else
                            ++p1;
                        while ( p2 < l2 && s2[p2] != '/' )
                            ++p2;
                        if ( p2 == l2 ) {
                            if ( p1 == l1 )
                                return true;
                            else
                                return false;
                        }
                        if ( s2[p2] != '/' )
                            return false;
                        else
                            ++p2;
                        continue;
                    }
                    if ( s2[p2] == '*' ) {
                        ++p2;
                        if ( p2 == l2 )
                            return true;
                        if ( s2[p2] != '/' )
                            return false;
                        else
                            ++p2;
                        while ( p1 < l1 && s1[p1] != '/' )
                            ++p1;
                        if ( p1 == l1 ) {
                            if ( p2 == l2 )
                                return true;
                            else
                                return false;
                        }
                        if ( s1[p1] != '/' )
                            return false;
                        else
                            ++p1;
                        continue;
                    }
                    return false;
                }
                return false;
            }
        }; // namespace core

    } // namespace core
} // namespace meisterwerk
