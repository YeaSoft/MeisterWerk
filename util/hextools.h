#pragma once

String hexByte( uint8_t byte ) {
    char b[3];
    itoa( byte, b, 16 );
    if ( byte < 16 )
        return "0" + String( b );
    else
        return String( b );
}
