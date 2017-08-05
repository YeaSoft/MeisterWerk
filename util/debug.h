// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

// a generic debug define (may also affectz 3rd party libraries)
#ifdef DEBUG
#define _MW_DEBUG 1
#endif

// the library specific debug define
#ifdef _MW_DEBUG

namespace meisterwerk {

    bool AssertFailedLine( const char *pszFileName, int nLine ) {
        Serial.print( "Assertion Failed: File " );
        Serial.print( pszFileName );
        Serial.print( ", Line " );
        Serial.println( String( nLine ) );
        return false;
    }

    void DebugBreak() {
    }
} // namespace meisterwerk

#define ASSERT( f )                                                                                                    \
    do {                                                                                                               \
        if ( !( f ) && meisterwerk::AssertFailedLine __FILE__, __LINE__ ) ) {                                          \
                meisterwerk::DebugBreak();                                                                             \
            }                                                                                                          \
    } while ( 0 )

#define VERIFY( f ) ASSERT( f )
#define DBG_ONLY( f ) f
#define DBG( f ) Serial.println( f )
#define DBGF( ... ) Serial.printf( __VA_ARGS__ )

#if _MW_DEBUG > 1
#define DBG2( f ) Serial.println( f )
#else
#define DBG2( f )
#endif

#else // _MW_DEBUG

#define ASSERT( f ) ( (void)( 0 ) )
#define VERIFY( f ) ( (void)( f ) )
#define DBG_ONLY( f )
#define DBG( f )
#define DBG2( f )
#define DBGF( ... )

#endif // !_MW_DEBUG
