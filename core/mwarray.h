// mwarray.h - The internal array class
//
// This is the declaration of the internal array
// class used for resource-management
// supports []-indexing, adding elements at the end and
// erasing an elements by index.
// no automatic reallocation (yet).

#pragma once

namespace meisterwerk {
    namespace core {

        template <typename T> class mwarray {
            private:
            T *arr;
            DBG_ONLY( unsigned int peakSize );
            unsigned int maxSize;
            unsigned int size;
            unsigned int arrPtr;

            public:
            mwarray( int maxArraySize ) {
                DBG_ONLY( peakSize = 0 );
                arrPtr  = 0;
                size    = 0;
                maxSize = maxArraySize;
                arr     = new T[maxArraySize];
            }

            ~mwarray() {
                if ( arr != nullptr ) {
                    // If size > 0 then there's a potential memory leak.
                    // This must be taken care of by the array owner.
                    delete[] arr;
                }
            }

            bool add( T ent ) {
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
            T operator[]( int i ) const {
                return *arr[i];
            }

            T &operator[]( int i ) {
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
