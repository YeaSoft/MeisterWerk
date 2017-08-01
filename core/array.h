// array.h - The internal array class
//
// This is the declaration of the internal array
// class used for resource-management
// supports []-indexing, adding elements at the end and
// erasing an elements by index.
// no automatic reallocation (yet).

#pragma once

#include <cassert>

namespace meisterwerk {
    namespace core {

        template <typename T> class array {
            private:
            T *arr;
            DBG_ONLY( unsigned int peakSize );
            unsigned int maxSize;
            unsigned int size;
            unsigned int arrPtr;

            public:
            array( int maxArraySize ) {
                DBG_ONLY( peakSize = 0 );
                arrPtr  = 0;
                size    = 0;
                maxSize = maxArraySize;
                arr     = new T[maxArraySize];
            }

            ~array() {
                if ( arr != nullptr ) {
                    delete[] arr;
                }
            }

            bool add( T &ent ) {
                if ( size >= maxSize ) {
                    return false;
                }
                arr[arrPtr] = ent;
                arrPtr      = arrPtr + 1;
                ++size;
                DBG_ONLY( if ( size > peakSize ) { peakSize = size; } );
                return true;
            }

            bool erase( unsigned int index ) {
                if ( index >= size ) {
                    return false;
                }
                for ( int i = index; i < size - 1; i++ ) {
                    arr[i] = arr[i + 1];
                }
                --size;
                --arrPtr;
                return true;
            }
            T operator[]( unsigned int i ) const {
                assert( i < size );
                return arr[i];
            }

            T &operator[]( unsigned int i ) {
                assert( i < size );
                return arr[i];
            }

            bool isEmpty() {
                if ( size == 0 )
                    return true;
                else
                    return false;
            }

            unsigned int length() {
                return ( size );
            }

            DBG_ONLY( unsigned int peak() { return ( peakSize ); } );
        };
    } // namespace core
} // namespace meisterwerk
