// crypt.h - A helper for symmetric encrytpion
// based on XXTEA algorithm.

namespace meisterwerk {
    namespace util {

        class crypt {
            public:
            //==CRC-CCITT=================================================================
            //
            // - Source: Dr.Dobb's, Bob Felice, June 17, 2007,
            //   http://www.drdobbs.com/implementing-the-ccitt-cyclical-redundan/199904926
            //
            //****************************************************************************
            //
            // crc16.c - generate a ccitt 16 bit cyclic redundancy check (crc)
            //
            //      The code in this module generates the crc for a block of data.
            //
            // crc16() - generate a 16 bit crc
            //
            //
            // PURPOSE
            //      This routine generates the 16 bit remainder of a block of
            //      data using the ccitt polynomial generator.
            //
            // CALLING SEQUENCE
            //      crc = crc16(data, len);
            //
            // PARAMETERS
            //      data    <-- address of start of data block
            //      len     <-- length of data block
            //
            // RETURNED VALUE
            //      crc16 value. data is calcuated using the 16 bit ccitt polynomial.
            //
            // NOTES
            //      The CRC is preset to all 1's to detect errors involving a loss
            //        of leading zero's.
            //      The CRC (a 16 bit value) is generated in LSB MSB order.
            //      Two ways to verify the integrity of a received message
            //        or block of data:
            //        1) Calculate the crc on the data, and compare it to the crc
            //           calculated previously. The location of the saved crc must be
            //           known.
            //         2) Append the calculated crc to the end of the data. Now calculate
            //           the crc of the data and its crc. If the new crc equals the
            //           value in "CRC_OK", the data is valid.
            //
            // PSEUDO CODE:
            //      initialize crc (-1)
            //      DO WHILE count NE zero
            //        DO FOR each bit in the data byte, from LSB to MSB
            //          IF (LSB of crc) EOR (LSB of data)
            //            crc := (crc / 2) EOR polynomial
            //          ELSE
            //            crc := (crc / 2)
            //          FI
            //        OD
            //      OD
            //      1's compliment and swap bytes in crc
            //      RETURN crc
            //
            //*************************************************************************
            uint16_t crc16( uint8_t *data_p, uint16_t length ) {
                uint8_t  i;
                uint16_t data;
                uint16_t crc;

                crc = 0xffff;

                if ( length == 0 )
                    return ( ~crc );

                do {
                    for ( i = 0, data = (unsigned int)0xff & *data_p++; i < 8; i++, data >>= 1 ) {
                        if ( ( crc & 0x0001 ) ^ ( data & 0x0001 ) )
                            crc = ( crc >> 1 ) ^ POLY;
                        else
                            crc >>= 1;
                    }
                } while ( --length );

                crc = ~crc;

                data = crc;
                crc  = ( crc << 8 ) | ( data >> 8 & 0xFF );

                return ( crc );
            }

            //==CRYPT-XXTEA==================================================================
            //
            //  - Source: http://en.wikipedia.org/wiki/XXTEA
            //

            #define DELTA 0x9e3779b9
            #define MX ( ( ( z >> 5 ^ y << 2 ) + ( y >> 3 ^ z << 4 ) ) ^ ( ( sum ^ y ) + ( key[( p & 3 ) ^ e] ^ z ) ) )

            //-------------------------------------------------------------------------------
            // According to Needham and Wheeler:
            //
            // BTEA will encode or decode n words as a single block where n > 1
            //
            // * v is the n word data vector
            // * k is the 4 word key
            // * assumes 32 bit 'long' and same endian coding and decoding
            //

            void xxtea_encrypt( uint32_t *v, int n, uint32_t const key[4] ) {
                uint32_t y, z, sum;
                uint16_t p, rounds, e;

                if ( n < 1 )
                    return;

                rounds = 6 + 52 / n;
                sum    = 0;
                z      = v[n - 1];
                do {
                    sum += DELTA;
                    e = ( sum >> 2 ) & 3;
                    for ( p = 0; p < n - 1; p++ ) {
                        y = v[p + 1];
                        z = v[p] += MX;
                    }
                    y = v[0];
                    z = v[n - 1] += MX;
                } while ( --rounds );
            }

            void xxtea_decrypt( uint32_t *v, int n, uint32_t const key[4] ) {
                uint32_t y, z, sum;
                uint16_t p, rounds, e;

                if ( n < 1 )
                    return;

                rounds = 6 + 52 / n;
                sum    = rounds * DELTA;
                y      = v[0];
                do {
                    e = ( sum >> 2 ) & 3;
                    for ( p = n - 1; p > 0; p-- ) {
                        z = v[p - 1];
                        y = v[p] -= MX;
                    }
                    z = v[n - 1];
                    y = v[0] -= MX;
                    sum -= DELTA;
                } while ( --rounds );
            }
        };
    } // namespace util
} // namespace meisterwerk
