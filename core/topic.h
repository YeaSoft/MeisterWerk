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

            bool match( String &mask ) const {
                return mqttmatch( c_str(), mask.c_str() );
            }

            bool match( const char *mask ) const {
                return mqttmatch( c_str(), mask );
            }

            String getFirst() const {
                // XXX todo
                return "";
            }

            String getSecond() const {
                // XXX todo
                return "";
            }

            String getThird() const {
                // XXX todo
                return "";
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
