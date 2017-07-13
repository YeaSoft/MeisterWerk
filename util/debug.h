#pragma once

#ifdef DEBUG
#define DBG( n ) Serial.println( n )
#else
#define DBG( n )
#endif
