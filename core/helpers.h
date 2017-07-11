#ifndef helpers_h
#define helpers_h

#ifdef DEBUG
#define DBG( n ) Serial.println( n )
#else
#define DBG( n )
#endif

unsigned long superdelta( unsigned long there, unsigned long now ) {
    return now > there ? now - there : ( (unsigned long)-1 ) - there + now;
}

#endif