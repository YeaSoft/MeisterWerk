// queue.h - The internal queue class
//
// This is the declaration of the internal queue
// class that is part of the implementation of the
// application method for non blocking communication
// between the components and scheduling

#ifndef queue_h
#define queue_h

namespace meisterwerk {
    namespace core {

        template <class T> class queue {
            private:
            T **         que;
            unsigned int maxSize;
            unsigned int size;
            unsigned int quePtr0;
            unsigned int quePtr1;

            public:
            queue( unsigned int maxQueueSize ) {
                quePtr0 = 0;
                quePtr1 = 0;
                size    = 0;
                maxSize = maxQueueSize;
                que     = (T **)malloc( sizeof( T * ) * maxSize );
                if ( que == nullptr )
                    maxSize = 0;
            }
            ~queue() {
                if ( que != nullptr ) {
                    // If size > 0 then there's a potential memory leak.
                    // This must be taken care of by the queue owner.
                    free( que );
                }
            }
            bool push( T *ent ) {
                if ( size >= maxSize )
                    return false;
                que[quePtr1] = ent;
                quePtr1      = ( quePtr1 + 1 ) % maxSize;
                ++size;
                return true;
            }
            T *pop() {
                if ( size == 0 )
                    return nullptr;
                T *pEnt = que[quePtr0];
                quePtr0 = ( quePtr0 + 1 ) % maxSize;
                --size;
                return pEnt;
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
        };
    }
}

#endif
