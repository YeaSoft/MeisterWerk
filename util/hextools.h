// Copyright Dominik Schloesser and Leo Moll 2017
// MIT License
//
// MeisterWerk IoT Framework
// https://github.com/YeaSoft/MeisterWerk/
// If you like this project, please add a star!

#pragma once

namespace meisterwerk {
    namespace util {

        String hexByte( uint8_t byte ) {
            char b[3];
            itoa( byte, b, 16 );
            if ( byte < 16 )
                return "0" + String( b );
            else
                return String( b );
        }
    }
}